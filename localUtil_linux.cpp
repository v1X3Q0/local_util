#include <string>
#include <stdint.h>
#include <map>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fstream>

#include "localUtil_linux.h"
#include "localUtil.h"

int gen_kallsymmap(std::map<std::string, uint64_t>* out_mapgen)
{
    int result = -1;
    char linebuffer[0x100];
    std::stringstream linestream;
    std::string linetmp;
    std::string stringtmp;
    int kallsym_index = 0;
    uint64_t address_parse = 0;
    std::string symname;
    std::ifstream proc_kallsyms;

    SAFE_PAIL(getuid() != 0, "non root process, can't parse kallsym\n");

    proc_kallsyms.open("/proc/kallsyms");
    
    while(proc_kallsyms.getline(linebuffer, sizeof(linebuffer), '\n'))
    {
        linetmp = std::string(linebuffer);
        linestream = std::stringstream(linetmp);
        linestream >> stringtmp;
        address_parse = strtoull((stringtmp.c_str()), NULL, 0x10);
        linestream >> stringtmp;
        linestream >> symname;
        (*out_mapgen)[symname] = address_parse;
    }
    
    result = 0;
fail:
    SAFE_FSCLOSE(proc_kallsyms);
    return result;
}
