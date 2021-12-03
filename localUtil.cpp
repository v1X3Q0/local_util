#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "localUtil.h"

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

template<typename unitLen>
void dumpMemT(unsigned char* base, size_t len, const char* format)
{
   size_t i = 0;
   printf("%04hx: ", (unsigned short)i);
   for (i = 0; i < len; i += (sizeof(unitLen)))
   {
      if (((i % 0x10) == 0) && (i != 0))
      {
         printf("\n");
         printf("UD:%04hx: ", (unsigned short)i);
      }
      printf(format, *((unitLen*)&base[i]));
   }
}

void dumpMem(uint8_t* base, size_t len, char format)
{
   printf("UD:");
   if (format == 's')
   {
      printf(" %s", (char*)base);
   }
   else
   {
      if (format == 'c')
      {
         dumpMemT<uint8_t>(base, len, "0x%02hhx ");
      }
      else if (format == 'h')
      {
         dumpMemT<uint16_t>(base, len, "0x%04hx ");
      }
      else if (format == 'w' || ((format == 0) && (sizeof(void*) == 4)))
      {
         dumpMemT<uint32_t>(base, len, "0x%08x ");
      }
      else if (format == 'q' || ((format == 0) && (sizeof(void*) == 8)))
      {
         dumpMemT<uint64_t>(base, len, "0x%016lx ");
      }
   }
   printf("\n");
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
