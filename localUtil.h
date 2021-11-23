#ifndef LOCALUTIL
#define LOCALUTIL

#define PAGE_SIZE   0x1000
#define PAGE_MASK   (PAGE_SIZE - 1)

#define DEFAULT_SEARCH_SIZE     0x80

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

size_t rstrnlen(const char* s, size_t maxlen);
size_t rfindnn(const char* s, size_t maxlen);
int rstrncmp(const char* s1, const char* s2, size_t maxlen);

#endif