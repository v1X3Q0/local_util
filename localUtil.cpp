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

void dumpMem(uint8_t* base, size_t len, char* format_a)
{
   char format = format_a[0];
   const char formatdump[10] = {0};
   char* formatdumpcur = (char*)formatdump;

   printf("UD:");
   if (format_a[1] == 'x')
   {
      sprintf(formatdumpcur, "0x");
      formatdumpcur = (char*)&formatdump[2];
   }

   if (format == 's')
   {
      printf(" %s", (char*)base);
   }
   else
   {
      if ((format == 'c') || (format == 'b'))
      {
         sprintf(formatdumpcur, "%%02hhx ");
         dumpMemT<uint8_t>(base, len, formatdump);
      }
      else if (format == 'h')
      {
         sprintf(formatdumpcur, "%%04hx ");
         dumpMemT<uint16_t>(base, len, formatdump);
      }
      else if (format == 'w' || ((format == 0) && (sizeof(void*) == 4)))
      {
         sprintf(formatdumpcur, "%%08x ");
         dumpMemT<uint32_t>(base, len, formatdump);
      }
      else if (format == 'q' || ((format == 0) && (sizeof(void*) == 8)))
      {
#ifdef _WIN32
         sprintf(formatdumpcur, "%%016llx ");
#else
         sprintf(formatdumpcur, "%%016lx ");
#endif
         dumpMemT<uint64_t>(base, len, formatdump);
      }
   }
   printf("\n");
}
