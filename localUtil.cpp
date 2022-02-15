#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "localUtil.h"

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
