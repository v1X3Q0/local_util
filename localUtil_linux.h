#ifndef LOCALUTIL_LINUX_H
#define LOCALUTIL_LINUX_H

#ifdef __cplusplus
extern "C"
{
#endif

void* redlsym(char* procBase, char* funcName);

#ifdef __cplusplus
}
#endif

#endif // LOCALUTIL_LINUX_H