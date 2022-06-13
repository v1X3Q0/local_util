#ifndef LOCAL_UTIL_WINDOWS_H
#define LOCAL_UTIL_WINDOWS_H
#include <Windows.h>

#define PE_HEAD_PTR 0x3c
#define COFF_OFFSET 0x4
#define OPT_OFFSET (COFF_OFFSET + sizeof(IMAGE_FILE_HEADER))
#define NT_HEADER_SIZE		4

#define SAFE_PWERROR(x, ...) \
    if (x) \
    { \
        printf("%s:%d ERROR:", __FILE__, __LINE__); \
        printLastError(); \
        printf(__VA_ARGS__); \
        goto fail; \
    }

#ifdef __cplusplus
extern "C"
{
#endif
void printLastError();
size_t ridlsym(UINT8* libBase, const char* libName, size_t symName);
size_t redlsym(UINT8* libBase, const char* symName, int virtual_address);
int nt_headsize(UINT8* libBase, size_t* nt_head_sz_out);
int section_with_sym(uint8_t* nt_header_tmp, size_t sym_address, IMAGE_SECTION_HEADER** section_64_out);
int get_pesection(UINT8* libBase, const char* section_name, IMAGE_SECTION_HEADER** section_a);
int pe_vatoraw(uint8_t* libBase, size_t symbol_va, void** symbol_out);
int isDriverLoaded(const wchar_t* targetLib, void** driver_out);
DWORD retEntryPoint(UINT8* targBase, DWORD* entryPoint);
#ifdef __cplusplus
}
#endif

#endif