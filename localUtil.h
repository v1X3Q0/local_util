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
        free(x); \
        x = 0; \
    }

#define SAFE_DEL(x) \
    if (x) \
    { \
        delete x; \
    }

size_t rstrnlen(const char* s, size_t maxlen);
size_t rfindnn(const char* s, size_t maxlen);
int rstrncmp(const char* s1, const char* s2, size_t maxlen);
void dumpMem(uint8_t* base, size_t len, char format);

#endif