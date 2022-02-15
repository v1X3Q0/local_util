#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "localUtil.h"

// reverse strlen
size_t rstrnlen(const char* s, size_t maxlen)
{
    for (size_t i = 0; i < maxlen; i++, s--)
    {
        if ((*s) == 0)
        {
            return i;
        }
        // generic non ascii character fail case
        if ((*s) > 0x7f)
        {
            return -1;
        }
    }
    return maxlen;
}

// reverse find next non-null character
size_t rfindnn(const char* s, size_t maxlen)
{
    for (size_t i = 0; i < maxlen; i++, s--)
    {
        if ((*s) != 0)
        {
            return i;
        }
    }
    return maxlen;
}

int rstrncmp(const char* s1, const char* s2, size_t maxlen)
{
    for (size_t i = 0; i < maxlen; i++, s1--, s2--)
    {
        if (*s1 != *s2)
        {
            return -1;
        }
        if (*s1 == 0)
        {
            break;
        }
    }
    return 0;
}

int block_grab(const char* fileTargName, void** allocBase, size_t* fSize)
{
    int result = -1;
    FILE* outFile = 0;
    size_t outfileSz = 0;
    size_t outfileSzPad = 0;
    void* allocTmp = 0;

    outFile = fopen(fileTargName, "r");
    SAFE_BAIL(outFile == 0);
    
    fseek(outFile, 0, SEEK_END);
    outfileSzPad = outfileSz = ftell(outFile);
    fseek(outFile, 0, SEEK_SET);

    if ((outfileSz % PAGE_SIZE4K) != 0)
    {
        outfileSzPad = (outfileSz + PAGE_SIZE4K) & ~PAGE_MASK4K;
    }

    posix_memalign(allocBase, PAGE_SIZE4K, outfileSzPad);

    fread(*allocBase, 1, outfileSz, outFile);

    if (fSize != 0)
    {
        *fSize = outfileSz;
    }

    result = 0;
fail:
    SAFE_FCLOSE(outFile);
    return result;
}

unsigned long subint(const char* strbase, size_t strsize, int radix)
{
    char* strbase_new = calloc(strsize + 1, 1);
    unsigned long strint = 0;

    strncpy(strbase_new, strbase, strsize);
    strint = strtoul(strbase_new, NULL, radix);

    SAFE_FREE(strbase_new);
    return strint;
}