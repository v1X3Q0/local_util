#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <elf.h>

#include "localUtil.h"

uint32_t elf_hash(const uint8_t* name) {
    uint32_t h = 0, g;
    for (; *name; name++) {
        h = (h << 4) + *name;
        if (g = h & 0xf0000000) {
            h ^= g >> 24;
        }
        h &= ~g;
    }
    return h;
}

/* Different architecture have different symbol structure size. */
/* typedef Elf32_Sym Elf64Sym_t; */

Elf64_Sym* elf_lookup(
    const char* strtab,      /* string table */
    Elf64_Sym* symtab,   /* symbol table */
    const uint32_t* hashtab, /* hash table */
    const char* symname      /* name to look up */
) {
    const uint32_t hash = elf_hash(symname);

    const uint32_t nbucket = hashtab[0];
    const uint32_t nchain = hashtab[1];
    const uint32_t* bucket = &hashtab[2];
    const uint32_t* chain = &bucket[nbucket];

    for (uint32_t i = bucket[hash % nbucket]; i; i = chain[i]) {
        if (strcmp(symname, strtab + symtab[i].st_name) == 0) {
            return &symtab[i];
        }
    }

    return NULL;
}

uint32_t gnu_hash(const uint8_t* name) {
    uint32_t h = 5381;

    for (; *name; name++) {
        h = (h << 5) + h + *name;
    }

    return h;
}

typedef uint64_t bloom_el_t;
#define ELFCLASS_BITS 64
/* 32-bit binary:
    typedef Elf32_Sym Elf64Sym_t;
    typedef bloom_el_t uint32_t;
    #define ELFCLASS_BITS 32
*/

Elf64_Sym* gnu_lookup(
    const char* strtab,      /* string table */
    Elf64_Sym* symtab,   /* symbol table */
    const uint32_t* hashtab,     /* hash table */
    const char* name         /* symbol to look up */
) {
    const uint32_t namehash = gnu_hash(name);

    const uint32_t nbuckets = hashtab[0];
    const uint32_t symoffset = hashtab[1];
    const uint32_t bloom_size = hashtab[2];
    const uint32_t bloom_shift = hashtab[3];
    const bloom_el_t* bloom = (void*)&hashtab[4];
    const uint32_t* buckets = (void*)&bloom[bloom_size];
    const uint32_t* chain = &buckets[nbuckets];

    bloom_el_t word = bloom[(namehash / ELFCLASS_BITS) % bloom_size];
    bloom_el_t mask = 0
        | (bloom_el_t)1 << (namehash % ELFCLASS_BITS)
        | (bloom_el_t)1 << ((namehash >> bloom_shift) % ELFCLASS_BITS);

    /* If at least one bit is not set, a symbol is surely missing. */
    if ((word & mask) != mask) {
        return NULL;
    }

    uint32_t symix = buckets[namehash % nbuckets];
    if (symix < symoffset) {
        return NULL;
    }

    /* Loop through the chain. */
    while (1) {
        const char* symname = strtab + symtab[symix].st_name;
        const uint32_t hash = chain[symix - symoffset];

        if (((namehash | 1) == (hash | 1)) && (strcmp(name, symname) == 0))
        {
            return &symtab[symix];
        }

        /* Chain ends with an element with the lowest bit set to 1. */
        if (hash & 1) {
            break;
        }

        symix++;
    }

    return NULL;
}

void* redlsym(char* procBase, char* funcName)
{
	Elf64_Ehdr* elfHead = 0;
	Elf64_Phdr* phdr = 0;
	int i = 0;
	size_t loadOff = 0xffffffffffffffff;
	Elf64_Dyn* dynTab = 0;
	Elf64_Dyn* dynEnt = 0;
	unsigned int* hashTab = 0;
	unsigned int* gnu_hashTab = 0;
	char* strTab = 0;
	Elf64_Sym* symTab = 0;
	Elf64_Sym* targetSym = 0;
    Elf64_Shdr* section_table = 0;
    const char* shstr_table = 0;
    void* result = 0;

    elfHead = (Elf64_Ehdr*)(procBase);
    phdr = (Elf64_Phdr*)(elfHead->e_phoff + procBase);

    section_table = (Elf64_Shdr*)(procBase + elfHead->e_shoff);
    shstr_table = procBase + section_table[elfHead->e_shstrndx].sh_offset;

	for (i = 0; i < elfHead->e_phnum; i++)
	{
		switch (phdr[i].p_type)
		{
			case PT_DYNAMIC:
				dynTab = (Elf64_Dyn*)(phdr[i].p_offset + procBase);
				break;
			case PT_LOAD:
				if (loadOff == 0xffffffffffffffff)
					loadOff = phdr[i].p_vaddr;
				else if (loadOff > phdr[i].p_vaddr)
					loadOff = phdr[i].p_vaddr;
				break;
		}
	}

	for (dynEnt = dynTab; dynEnt->d_tag != DT_NULL; dynEnt++)
	{
		switch (dynEnt->d_tag)
		{
			case DT_HASH:
				hashTab = (unsigned int*)(dynEnt->d_un.d_val + procBase);
				break;
			case DT_GNU_HASH:
				gnu_hashTab = (unsigned int*)(dynEnt->d_un.d_val + procBase);
				break;
			case DT_STRTAB:
				strTab = (char*)(dynEnt->d_un.d_val + procBase);
				break;
			case DT_SYMTAB:
				symTab = (Elf64_Sym*)(dynEnt->d_un.d_val + procBase);
				break;
		}
	}

	if (gnu_hashTab != 0)
    {
		targetSym = gnu_lookup(strTab, symTab, gnu_hashTab, funcName);
    }
	else
    {
		targetSym = elf_lookup(strTab, symTab, hashTab, funcName);
    }
	if (targetSym != 0)
    {
		result = (void*)targetSym->st_value;
    }
    else
    {

    }
	return result;
}

int elf_vatoraw(uint8_t* libBase, size_t symbol_va, size_t* symbol_out)
{
    int result = -1;
    Elf64_Ehdr* ehdr = 0;
    Elf64_Phdr* phdr = 0;
    int i = 0;
    int offset_tracked = 0;

    ehdr = (Elf64_Ehdr*)(libBase);
    phdr = (Elf64_Phdr*)(libBase + ehdr->e_phoff);

    for (i = 0; i < ehdr->e_phnum; i++)
    {
        FINISH_IF(REGION_CONTAINS(phdr[i].p_vaddr, phdr[i].p_memsz, symbol_va) == 1);        
    }

    goto fail;
finish:
    result = 0;
    if (symbol_out != 0)
    {
        *symbol_out = (symbol_va - phdr[i].p_vaddr) + phdr[i].p_offset;
    }

fail:
    return result;
}
