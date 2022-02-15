#ifndef LOCALUTIL
#define LOCALUTIL

#include <stdint.h>
#include <stdio.h>

#define DEFAULT_SEARCH_SIZE     0x80

#define PAGE_SIZE4K   0x1000
#define PAGE_MASK4K   (PAGE_SIZE4K - 1)

#ifdef __linux__
#ifndef PAGE_SIZE
#define PAGE_SIZE PAGE_SIZE4K
#endif
#ifndef PAGE_MASK
#define PAGE_MASK PAGE_MASK4K
#endif
#endif

#define CASE_OVERLAP_R1_IN_R2(reg1, reg1sz, reg2, reg2sz) \
    ((reg1 >= reg2) && (reg1 < (reg2 + reg2sz)))
    
#define CASE_OVERLAP_R1_ENDS_R2(reg1, reg1sz, reg2, reg2sz) \
    (((reg1 + reg1sz) >= reg2) && ((reg1 + reg1sz) < (reg2 + reg2sz)))

#define CASE_OVERLAP_R1_EATS_R2(reg1, reg1sz, reg2, reg2sz) \
    ((reg1 < reg2) && ((reg1 + reg1sz) >= (reg2 + reg2sz)))

// first case, reg1 is inside of reg2
// second case, reg1 ends in reg2
// final case, reg1 encompasses reg2 
#define OVERLAP_REGIONS(reg1, reg1sz, reg2, reg2sz) \
    CASE_OVERLAP_R1_IN_R2(reg1, reg1sz, reg2, reg2sz) || \
    CASE_OVERLAP_R1_ENDS_R2(reg1, reg1sz, reg2, reg2sz) || \
    CASE_OVERLAP_R1_EATS_R2(reg1, reg1sz, reg2, reg2sz)
    
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

// #define SAFE_BAIL(x) \
//     if (x) \
//     { \
//         printf("%s:%d\n", __FILE__, __LINE__); \
//         goto fail; \
//     }

#define SAFE_CONT(x) \
    if (x) \
    { \
        continue; \
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

#define SAFE_CLOSE(x) \
    if (x) \
    { \
        close(x); \
        x = 0; \
    }

#define SAFE_FCLOSE(x) \
    if (x) \
    { \
        fclose(x); \
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

#define BIT_PAD(x, TYPE_AUTO, PAD_TO) \
    if (((size_t)x % PAD_TO) != 0) \
    { \
        x = (TYPE_AUTO)((size_t)x + PAD_TO - ((size_t)x % PAD_TO)); \
    }

size_t rstrnlen(const char* s, size_t maxlen);
size_t rfindnn(const char* s, size_t maxlen);
int rstrncmp(const char* s1, const char* s2, size_t maxlen);
int block_grab(const char* fileTargName, void** allocBase, size_t* fSize);


#endif