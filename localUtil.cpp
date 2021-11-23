#include <string.h>

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