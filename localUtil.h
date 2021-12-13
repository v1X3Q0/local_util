#ifndef LOCALUTIL
#define LOCALUTIL

#include <stdint.h>
#include <stdio.h>

#include <vector>
#include <algorithm>

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
        x = 0; \
    }

size_t rstrnlen(const char* s, size_t maxlen);
size_t rfindnn(const char* s, size_t maxlen);
int rstrncmp(const char* s1, const char* s2, size_t maxlen);
void dumpMem(uint8_t* base, size_t len, char format);
int block_grab(const char* fileTargName, void** allocBase, size_t* fSize);

template<typename t, typename u>
int vector_pair_ind(std::vector<std::pair<t, u>>* toSearch, t lookupKey)
{
    int indexOut = 0;

    for (auto i = toSearch->begin(); i != toSearch->end(); i++)
    {
        if (i->first == lookupKey)
        {
            return indexOut;
        }
        indexOut++;
    }
    return -1;
}

template<typename t, typename u>
u vector_pair_key_find(std::vector<std::pair<t, u>>* targetVector, t lookupKey)
{
    int newInd = vector_pair_ind<t, u>(targetVector, lookupKey);
    if (newInd == -1)
    {
        return 0;
    }
    else
    {
        return (*targetVector)[newInd].second;
    }
}

template<typename t, typename u>
void vector_pair_sort(std::vector<std::pair<t, u>>* targetVector,
    bool (*cmp)(std::pair<t, u>&, std::pair<t, u>&))
{
    std::sort(targetVector->begin(), targetVector->end(), cmp);
}


#endif