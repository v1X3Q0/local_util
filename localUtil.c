#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <dirent.h>
#include <libgen.h>
#endif

#ifdef __linux__ 
#include <linux/limits.h>
#elif defined(__APPLE__)
#include <sys/syslimits.h>
#endif

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

#ifdef _WIN32
    *allocBase = VirtualAlloc(NULL, PAGE_SIZE4K, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    posix_memalign(allocBase, PAGE_SIZE4K, outfileSzPad);
#endif
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

int recurse_op(int (*routine_on_file)(const char*, int, void**), const char* path_dir, int count, void** vargs)
{
    int result = -1;
#ifndef _WIN32
    struct dirent *de; // Pointer for directory entry
    char path_tmp[PATH_MAX] = { 0 };
    DIR *dr = 0;
    struct stat fileStat = {0};

    // printf("%s\n", path_dir);

    SAFE_BAIL(stat(path_dir, &fileStat) < 0)
    // if((fileStat.st_mode & __S_IFMT) == __S_IFREG)
    if(S_ISREG(fileStat.st_mode) != 0)
    {
        result = routine_on_file(path_dir, count, vargs);
        goto finish;
    }

    // opendir() returns a pointer of DIR type.
    dr = opendir(path_dir);
    SAFE_BAIL(dr == NULL);

    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    // for readdir()
    while ((de = readdir(dr)) != NULL)
    {
        if ((strcmp(de->d_name, ".") == 0) || (strcmp(de->d_name, "..") == 0))
        {
            continue;
        }

        memset(path_tmp, 0, sizeof(path_tmp));
        strcpy(path_tmp, path_dir);
        strcat(path_tmp, "/");
        strcat(path_tmp, de->d_name);

        if (de->d_type == DT_REG)
        {
            routine_on_file(path_tmp, count, vargs);
        }
        else if (de->d_type == DT_DIR)
        {
            recurse_op(routine_on_file, path_tmp, count, vargs);
        }
    }


finish:
    SAFE_CLOSEDIR(dr);
    result = 0;
fail:
#endif
    return result;
}
