#include <stdio.h>
#include <Windows.h>

#include <psapi.h>
#include <tchar.h>

#include "localUtil.h"
#include "localUtil_windows.h"

#define ARRAY_SIZE 1024

void printLastError()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = 0;
    LPSTR messageBuffer = NULL;
    size_t size = 0;

    errorMessageID = GetLastError();

    size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    
    printf("%d:%s\n", errorMessageID, messageBuffer);
    LocalFree(messageBuffer);
}

void dumpData(size_t* dataBase, size_t dbSz)
{
	for (int i = 0; (i * sizeof(size_t)) < dbSz; i++)
	{
		if ((i % 4) == 0)
		{
			printf("\n");
		}

		printf("0x%016llx ", dataBase[i]);
	}
	printf("\n");
}

LPVOID isDriverLoaded(const wchar_t* targetLib)
{
	LPVOID drivers[ARRAY_SIZE];
	DWORD cbNeeded;
	int cDrivers, i;
	TCHAR szDriver[ARRAY_SIZE];

	SAFE_BAIL(EnumDeviceDrivers(drivers, sizeof(drivers), &cbNeeded) && cbNeeded >= sizeof(drivers));

	cDrivers = cbNeeded / sizeof(drivers[0]);

	//_tprintf(TEXT("There are %d drivers:\n"), cDrivers);
	for (i = 0; i < cDrivers; i++)
	{
		if (GetDeviceDriverBaseName(drivers[i], szDriver, sizeof(szDriver) / sizeof(szDriver[0])))
		{
			if (wcscmp(szDriver, targetLib) == 0)
			{
				return drivers[i];
			}
			//_tprintf(TEXT("%d: %s\n"), i + 1, szDriver);
		}
	}

fail:
	//_tprintf(TEXT("EnumDeviceDrivers failed; array size needed is %x\n"), cbNeeded / sizeof(LPVOID));
	return 0;
}

LPVOID GetModuleBaseAddress(const TCHAR* targetLib)
{
	DWORD processID = GetCurrentProcessId();
	HANDLE procHand = GetCurrentProcess();
    LPVOID baseAddress = 0;
    HANDLE      processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    HMODULE* moduleArray = 0;
    LPBYTE      moduleArrayBytes = 0;
    DWORD       bytesRequired = 0;
	unsigned int moduleCount = 0;
	TCHAR curModName[MAX_PATH];
	DWORD i = 0;
	MODULEINFO modCurrent = { 0 };

    SAFE_BAIL(processHandle == 0);

    SAFE_BAIL(EnumProcessModules(processHandle, NULL, 0, &bytesRequired) == 0);

    SAFE_BAIL(bytesRequired == 0);

    moduleArrayBytes = (LPBYTE)LocalAlloc(LPTR, bytesRequired);

	SAFE_BAIL(moduleArrayBytes == 0);

	moduleCount = bytesRequired / sizeof(HMODULE);
	moduleArray = (HMODULE*)moduleArrayBytes;

	SAFE_BAIL(EnumProcessModules(processHandle, moduleArray, bytesRequired, &bytesRequired) == 0);
	
	for (i = 0; i < moduleCount; i++)
	{
		GetModuleBaseName(procHand, moduleArray[i], curModName, MAX_PATH);
		if (wcscmp(curModName, targetLib) == 0)
		{
			GetModuleInformation(procHand, moduleArray[i], &modCurrent, sizeof(MODULEINFO));
			baseAddress = modCurrent.lpBaseOfDll;
			goto fail;
		}
	}

fail:

	SAFE_LFREE(moduleArrayBytes);

	SAFE_HCLOSE(processHandle);

    return baseAddress;
}

DWORD retEntryPoint(UINT8* targBase)
{
	DWORD pe_base = 0;
	IMAGE_OPTIONAL_HEADER* opt_head = 0;
	DWORD addrEntry = 0;

	SAFE_BAIL(*(UINT16*)(&targBase[0]) != IMAGE_DOS_SIGNATURE);
	pe_base = *(DWORD*)(&targBase[PE_HEAD_PTR]);
	SAFE_BAIL(*(DWORD*)(&(targBase[pe_base])) != IMAGE_NT_SIGNATURE);
	//Get headers
	opt_head = (IMAGE_OPTIONAL_HEADER*)&targBase[pe_base + OPT_OFFSET];
	addrEntry = opt_head->AddressOfEntryPoint;
fail:
	return addrEntry;
}

size_t rdlsym(UINT8* libBase, const char* libName, size_t symName)
{
	DWORD pe_base = 0;
	IMAGE_OPTIONAL_HEADER* opt_head = 0;
	DWORD addrEntry = 0;
	IMAGE_THUNK_DATA* ogT = 0;
	IMAGE_THUNK_DATA* ngT = 0;
	int num_of_imports = 0;
	IMAGE_IMPORT_BY_NAME* import_name = 0;
	HMODULE temp_Lib = 0;
	size_t result = 0;
	char* nameTemp = 0;
	IMAGE_IMPORT_DESCRIPTOR* import_descriptors = 0;
	IMAGE_THUNK_DATA* thunk_i_can = 0;

	SAFE_BAIL(*(UINT16*)(&libBase[0]) != IMAGE_DOS_SIGNATURE);
	pe_base = *(DWORD*)(&libBase[PE_HEAD_PTR]);
	SAFE_BAIL(*(DWORD*)(&(libBase[pe_base])) != IMAGE_NT_SIGNATURE);
	//Get headers
	opt_head = (IMAGE_OPTIONAL_HEADER*)&libBase[pe_base + OPT_OFFSET];

	import_descriptors = (IMAGE_IMPORT_DESCRIPTOR*)(libBase + opt_head->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	num_of_imports = opt_head->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size / sizeof(IMAGE_IMPORT_DESCRIPTOR) - 1;
	thunk_i_can = (IMAGE_THUNK_DATA*)(opt_head->DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress + libBase);

	for (int i = 0; i < num_of_imports; i++, import_descriptors++)
	{
		nameTemp = (char*)(import_descriptors->Name + libBase);
		if (strcmp(nameTemp, libName) != 0)
		{
			continue;
		}

		ogT = (IMAGE_THUNK_DATA*)(import_descriptors->OriginalFirstThunk + libBase);
		ngT = (IMAGE_THUNK_DATA*)(import_descriptors->FirstThunk + libBase);
		while (ogT->u1.AddressOfData != 0)
		{
			if (ogT->u1.AddressOfData & IMAGE_ORDINAL_FLAG)
			{
				if ((size_t)(ogT->u1.Ordinal % (2 << 15)) == symName)
				{
					result = ogT->u1.Function;
					goto finish;
				}
			}
			else
			{
				import_name = (IMAGE_IMPORT_BY_NAME*)(ogT->u1.AddressOfData + libBase);
				if (strcmp((LPCSTR)import_name->Name, (const char*)symName) == 0)
				{
					result = ogT->u1.Function;
					goto finish;
				}
			}
			ngT++;
			ogT++;
		}
	}
	goto finish;
fail:

finish:
	return result;
}
