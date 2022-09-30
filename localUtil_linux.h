#ifndef LOCALUTIL_LINUX_H
#define LOCALUTIL_LINUX_H

#ifdef __cplusplus
extern "C"
{
int gen_kallsymmap(std::map<std::string, uint64_t>* out_mapgen);
#endif

void* redlsym(char* procBase, char* funcName);
int elf_vatoraw(uint8_t* libBase, size_t symbol_va, size_t* symbol_out);
#define virt_to_file elf_vatoraw

#ifdef __cplusplus
}
#endif

#endif // LOCALUTIL_LINUX_H