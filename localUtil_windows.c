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

int isDriverLoaded(const wchar_t* targetLib, void** driver_out)
{
	int result = -1;
	LPVOID drivers[ARRAY_SIZE];
	DWORD cbNeeded;
	int cDrivers, i;
	TCHAR szDriver[ARRAY_SIZE];

	SAFE_BAIL(EnumDeviceDrivers(drivers, sizeof(drivers), &cbNeeded) && cbNeeded >= sizeof(drivers));

	cDrivers = cbNeeded / sizeof(drivers[0]);

	//_tprintf(TEXT("There are %d drivers:\n"), cDrivers);
	for (i = 0; i < cDrivers; i++)
	{
		if (GetDeviceDriverBaseNameW(drivers[i], szDriver, sizeof(szDriver) / sizeof(szDriver[0])))
		{
			FINISH_IF(wcscmp(szDriver, targetLib) == 0);
			//_tprintf(TEXT("%d: %s\n"), i + 1, szDriver);
		}
	}
	goto fail;
finish:
	result = 0;
	if (driver_out != 0)
	{
		*driver_out = drivers[i];
	}
fail:
	//_tprintf(TEXT("EnumDeviceDrivers failed; array size needed is %x\n"), cbNeeded / sizeof(LPVOID));
	return result;
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

DWORD retEntryPoint(UINT8* targBase, DWORD* entryPoint)
{
	int result = -1;
	DWORD pe_base = 0;
	IMAGE_OPTIONAL_HEADER* opt_head = 0;
	DWORD addrEntry = 0;

	SAFE_BAIL(*(UINT16*)(&targBase[0]) != IMAGE_DOS_SIGNATURE);
	pe_base = *(DWORD*)(&targBase[PE_HEAD_PTR]);
	SAFE_BAIL(*(DWORD*)(&(targBase[pe_base])) != IMAGE_NT_SIGNATURE);
	//Get headers
	opt_head = (IMAGE_OPTIONAL_HEADER*)&targBase[pe_base + OPT_OFFSET];
	addrEntry = opt_head->AddressOfEntryPoint;
	
	result = 0;
	if (entryPoint != 0)
	{
		*entryPoint = addrEntry;
	}
fail:
	return result;
}

// take a library base, look for an import from a library and then return the
// resolution. Can be used if you have a library loaded in memory and want to
// find the address of something that it imports.
size_t ridlsym(UINT8* libBase, const char* libName, size_t symName)
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

int nt_headsize(UINT8* libBase, size_t* nt_head_sz_out)
{
	int result = -1;
	IMAGE_OPTIONAL_HEADER* opt_head = 0;
	IMAGE_FILE_HEADER* coff_head = 0;
	DWORD pe_head = 0;
	IMAGE_SECTION_HEADER* section = 0;

	// get the .edata section
	SAFE_BAIL(*(UINT16*)(&libBase[0]) != IMAGE_DOS_SIGNATURE);
	pe_head = *(DWORD*)(&libBase[PE_HEAD_PTR]);
	SAFE_BAIL(*(DWORD*)(&(libBase[pe_head])) != IMAGE_NT_SIGNATURE);
	//Get headers
	coff_head = (IMAGE_FILE_HEADER*)&libBase[pe_head + COFF_OFFSET];
	opt_head = (IMAGE_OPTIONAL_HEADER*)&libBase[pe_head + OPT_OFFSET];

	result = 0;
	if (nt_head_sz_out != 0)
	{
		*nt_head_sz_out = opt_head->SizeOfHeaders;
	}
fail:
	return result;
}

int get_pesection(UINT8* libBase, const char* section_name, IMAGE_SECTION_HEADER** section_a)
{
	int result = -1;
	IMAGE_OPTIONAL_HEADER* opt_head = 0;
	IMAGE_FILE_HEADER* coff_head = 0;
	DWORD pe_head = 0;
	IMAGE_SECTION_HEADER* section = 0;

	// get the .edata section
	SAFE_BAIL(*(UINT16*)(&libBase[0]) != IMAGE_DOS_SIGNATURE);
	pe_head = *(DWORD*)(&libBase[PE_HEAD_PTR]);
	SAFE_BAIL(*(DWORD*)(&(libBase[pe_head])) != IMAGE_NT_SIGNATURE);
	//Get headers
	coff_head = (IMAGE_FILE_HEADER*)&libBase[pe_head + COFF_OFFSET];
	opt_head = (IMAGE_OPTIONAL_HEADER*)&libBase[pe_head + OPT_OFFSET];
	section = (IMAGE_SECTION_HEADER*)(libBase + pe_head + coff_head->SizeOfOptionalHeader + sizeof(IMAGE_FILE_HEADER) + NT_HEADER_SIZE);

	for (int i = 0; i < coff_head->NumberOfSections; i++, section++)
	{
		FINISH_IF(strcmp((const char*)section_name, section->Name) == 0)
	}
	goto fail;
finish:
	result = 0;
	if (section_a != 0)
	{
		*section_a = section;
	}
fail:
	return result;
}

#define VIRTUAL_ADJUST(VA_TRUE, RES, REBASE, RVA, VIRT, RAW) \
	if (VA_TRUE == FALSE) \
	{ \
		RES = REBASE + RVA - VIRT + RAW; \
	} \
	else \
	{ \
		RES = REBASE + RVA; \
	}

int pe_vatoraw(uint8_t* libBase, size_t symbol_va, void** symbol_out)
{
	int result = -1;
	size_t symTmp = symbol_va;
	IMAGE_SECTION_HEADER* section_64_static = 0;

	// on the read file
	SAFE_BAIL(section_with_sym(libBase, symTmp, &section_64_static) == -1);

	symTmp = symTmp - section_64_static->VirtualAddress + section_64_static->PointerToRawData;

	result = 0;
	if (symbol_out != 0)
	{
		*symbol_out = (void*)symTmp;
	}
fail:
	return result;
}


size_t redlsym(UINT8* libBase, const char* symName, int virtual_address)
{
	size_t result = 0;
	IMAGE_SECTION_HEADER* sections = 0;
	IMAGE_EXPORT_DIRECTORY* export_directory = 0;
	DWORD SRA = 0;
	DWORD SVA = 0;
	uint32_t* addrNames = 0;
	const char* funcNameAddr = 0;
	uint16_t* addrOrd = 0;
	uint16_t funcOrd = 0;
	uint32_t* addrFunc = 0;
	size_t funcAddr = 0;
	int nameOrd = 0;

	SAFE_BAIL(get_pesection(libBase, ".edata", &sections) == -1);

	// found our section, we had to because if its raw we're fucked
	SRA = sections->PointerToRawData;
	SVA = sections->VirtualAddress;
	export_directory = (IMAGE_EXPORT_DIRECTORY*)(libBase + sections->VirtualAddress);
	if (virtual_address == FALSE)
	{
		export_directory = (IMAGE_EXPORT_DIRECTORY*)(libBase + sections->PointerToRawData);
	}
	VIRTUAL_ADJUST(virtual_address, addrNames, libBase, export_directory->AddressOfNames, SVA, SRA);
	VIRTUAL_ADJUST(virtual_address, addrOrd, libBase, export_directory->AddressOfNameOrdinals, SVA, SRA);
	VIRTUAL_ADJUST(virtual_address, addrFunc, libBase, export_directory->AddressOfFunctions, SVA, SRA);

	// proceed to use the .edata section
	for (; nameOrd < export_directory->NumberOfNames; nameOrd++)
	{
		VIRTUAL_ADJUST(virtual_address, funcNameAddr, libBase, *(uint32_t*)(addrNames + nameOrd), SVA, SRA);
		// funName = readString
		FINISH_IF(strcmp(funcNameAddr, symName) == 0);
	}
	goto fail;
finish:
	funcOrd = *(uint16_t*)(addrOrd + nameOrd);
	funcAddr = *(uint32_t*)(addrFunc + funcOrd);
	result = funcAddr;
fail:
	return result;
}

int section_with_sym(uint8_t* nt_header_tmp, size_t sym_address, IMAGE_SECTION_HEADER** section_64_out)
{
	int result = -1;
	IMAGE_OPTIONAL_HEADER* opt_head = 0;
	IMAGE_FILE_HEADER* coff_head = 0;
	DWORD pe_head = 0;
	IMAGE_SECTION_HEADER* section = 0;

	// get the .edata section
	SAFE_BAIL(*(UINT16*)(&nt_header_tmp[0]) != IMAGE_DOS_SIGNATURE);
	pe_head = *(DWORD*)(&nt_header_tmp[PE_HEAD_PTR]);
	SAFE_BAIL(*(DWORD*)(&(nt_header_tmp[pe_head])) != IMAGE_NT_SIGNATURE);
	//Get headers
	coff_head = (IMAGE_FILE_HEADER*)&nt_header_tmp[pe_head + COFF_OFFSET];
	opt_head = (IMAGE_OPTIONAL_HEADER*)&nt_header_tmp[pe_head + OPT_OFFSET];
	section = (IMAGE_SECTION_HEADER*)(nt_header_tmp + pe_head + coff_head->SizeOfOptionalHeader + sizeof(IMAGE_FILE_HEADER) + NT_HEADER_SIZE);

	for (int i = 0; i < coff_head->NumberOfSections; i++, section++)
	{
		FINISH_IF(REGION_CONTAINS(section->VirtualAddress, section->Misc.VirtualSize, sym_address) == TRUE);
	}

	goto fail;
finish:
	result = 0;
	if (section_64_out != 0)
	{
		*section_64_out = section;
	}
fail:
	return result;
}
