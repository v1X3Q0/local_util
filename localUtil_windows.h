#ifndef LOCAL_UTIL_WINDOWS_H
#define LOCAL_UTIL_WINDOWS_H

#define PE_HEAD_PTR 0x3c
#define COFF_OFFSET 0x4
#define OPT_OFFSET (COFF_OFFSET + sizeof(IMAGE_FILE_HEADER))

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
#ifdef __cplusplus
}
#endif

#endif