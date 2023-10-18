#ifndef LOCALUTIL
#define LOCALUTIL

#include <stdint.h>
#include <stdio.h>

#define DEFAULT_SEARCH_SIZE     0x80

#define PAGE_SIZE4K   0x1000
#define PAGE_MASK4K   (PAGE_SIZE4K - 1)

// #ifdef __linux__
#ifndef PAGE_SIZE
#define PAGE_SIZE PAGE_SIZE4K
#endif
#ifndef PAGE_MASK
#define PAGE_MASK PAGE_MASK4K
#endif
// #endif

#define CASE_OVERLAP_R1_IN_R2(reg1, reg1sz, reg2, reg2sz) \
    ((reg1 >= reg2) && (reg1 < (reg2 + reg2sz)))
    
#define CASE_OVERLAP_R1_ENDS_R2(reg1, reg1sz, reg2, reg2sz) \
    (((reg1 + reg1sz) > reg2) && ((reg1 + reg1sz) < (reg2 + reg2sz)))

// this case is if reg1 is less than reg2, and then gets to or passed it.
#define CASE_OVERLAP_R1_EATS_R2(reg1, reg1sz, reg2, reg2sz) \
    ((reg1 < reg2) && ((reg1 + reg1sz) > (reg2 + reg2sz)))

// first case, reg1 is inside of reg2
// second case, reg1 ends in reg2
// final case, reg1 encompasses reg2 
#define OVERLAP_REGIONS(reg1, reg1sz, reg2, reg2sz) \
    CASE_OVERLAP_R1_IN_R2(reg1, reg1sz, reg2, reg2sz) || \
    CASE_OVERLAP_R1_ENDS_R2(reg1, reg1sz, reg2, reg2sz) || \
    CASE_OVERLAP_R1_EATS_R2(reg1, reg1sz, reg2, reg2sz)
    
#define REGION_CONTAINS(reg, regSz, targ_addr) \
    CASE_OVERLAP_R1_IN_R2(targ_addr, 0, reg, regSz)

#define FINISH_IF(x) \
    if (x) \
    { \
        goto finish; \
    }

#define SAFE_BAIL(x) \
    if (x) \
    { \
        goto fail; \
    }

#define SAFE_CLOSESOCKET(x) \
    if (x) \
    { \
        closesocket(x); \
        x = 0; \
    }

#define SAFE_EXIT(x) \
    if (x) \
    { \
        exit(0); \
    }

#define SAFE_FSCLOSE(x) \
    if (x.is_open() == true) \
    { \
        x.close(); \
    }

#define SAFE_PEXIT(x, ...) \
    if (x) \
    { \
        printf(__VA_ARGS__); \
        exit(0); \
    }

#define SAFE_CONT(x) \
    if (x) \
    { \
        continue; \
    }

#define SAFE_BREAK(x) \
    if (x) \
    { \
        break; \
    }

#define SAFE_PBREAK(x, ...) \
    if (x) \
    { \
        printf(__VA_ARGS__); \
        break; \
    }

#define SAFE_PAIL(x, ...) \
    if (x) \
    { \
        printf(__VA_ARGS__); \
        goto fail; \
    }

#define SAFE_FAIL(x, ...) \
    if (x) \
    { \
        fprintf(stderr, "ERROR %s:%d ", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
        goto fail; \
    }

#define SAFE_ERR(x) \
    if (x) \
    { \
        fprintf(stderr, "ERROR %s:%d ", __FILE__, __LINE__); \
        goto fail; \
    }

#define SAFE_CLOSE(x) \
    if (x > -1) \
    { \
        close(x); \
        x = -1; \
    }

#define SAFE_FCLOSE(x) \
    if (x) \
    { \
        fclose(x); \
        x = 0; \
    }

#define SAFE_HCLOSE(x) \
    if ((x) != INVALID_HANDLE_VALUE) \
    { \
        CloseHandle(x); \
        x = INVALID_HANDLE_VALUE; \
    }

#define SAFE_FREELIB(x) \
    if ((x) != NULL) \
    { \
        FreeLibrary(x); \
        x = 0; \
    }

#define SAFE_LFREE(x) \
    if (x) \
    { \
        LocalFree(x); \
        x = 0; \
    }

#define SAFE_FREE(x) \
    if (x) \
    { \
        free((void*)x); \
        x = 0; \
    }

#define SAFE_DEL(x) \
    if (x) \
    { \
        delete x; \
        x = 0; \
    }

#define SAFE_DELA(x) \
    if (x) \
    { \
        delete[] x; \
        x = 0; \
    }

#define SAFE_CLOSEDIR(x) \
    if (x) \
    { \
        closedir(x); \
        x = 0; \
    }

#define SAFE_REPOINT(x, y) \
    if (x != 0) \
    { \
        *x = y; \
    }

#define BIT_PAD(x, TYPE_AUTO, PAD_TO) \
    if (((size_t)x % PAD_TO) != 0) \
    { \
        x = (TYPE_AUTO)((size_t)x + PAD_TO - ((size_t)x % PAD_TO)); \
    }

#ifdef _WIN32
#define OSHANDLE HANDLE
#define OPENREAD(fname) CreateFileA(fname, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)
// file to open
// open for reading
// share for reading
// default security
// existing file only
// overlapped operation
// no attr. template
#define OSSEEK(handle, SEEK_DIR) SetFilePointer(handle, 0, 0, SEEK_DIR)
#define OSFSZ(handle) GetFileSize(handle, NULL)
#define OSBA(newAlloc, SZ_ALLOC) \
    newAlloc = VirtualAlloc(NULL, SZ_ALLOC, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
#define SAFE_OSFCLOSE SAFE_HCLOSE
#define OSREAD(oshandle, INBUF, NBYTES, OUTCOUNT) ReadFile(oshandle, INBUF, NBYTES, &OUTCOUNT, NULL);
#else
#define OSHANDLE FILE*
#define OPENREAD(fname) fopen(fname, "r")
#define OSSEEK(handle, SEEK_DIR) fseek(handle, 0, SEEK_DIR)
#define OSFSZ(handle) ftell(handle)
#ifdef __METALKIT__
#define OSBA(newAlloc, SZ_ALLOC) \
    newAlloc = memalign(PAGE_SIZE4K, SZ_ALLOC)
#else
#define OSBA(newAlloc, SZ_ALLOC) \
    posix_memalign(&newAlloc, PAGE_SIZE4K, SZ_ALLOC)
#endif // __METALKIT__
#define SAFE_OSFCLOSE SAFE_FCLOSE
#define OSREAD(oshandle, INBUF, NBYTES, OUTCOUNT) fread(INBUF, 1, NBYTES, oshandle)
#endif


#ifdef __cplusplus
extern "C" {
#endif

size_t rstrnlenu(const char* s, size_t maxlen);
size_t strnlenu(const char* s, size_t maxlen);
size_t rfindnn(const char* s, size_t maxlen);
int rstrncmp(const char* s1, const char* s2, size_t maxlen);
int block_grab(const char* fileTargName, void** allocBase, size_t* fSize);
unsigned long subint(const char* strbase, size_t strsize, int radix);
int recurse_op(int (*routine_on_file)(const char*, int, void**),
    const char* path_dir, int count, void** vargs);

#ifdef __cplusplus
}
#endif

#endif