#include <mach-o/loader.h>
#include <mach-o/getsect.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <pwd.h>
#include <string.h>

#include "localUtil.h"
#include "localUtil_xnu.h"

int getloadcommandfrommach(struct mach_header_64* mach_header_tmp, uint32_t lc_targ, struct load_command** lc_res)
{
    int result = -1;
    int cmd_index = 0;
    struct load_command* lc_iter = (struct load_command*)&mach_header_tmp[1];

    for (; cmd_index < mach_header_tmp->ncmds; cmd_index++)
    {
        FINISH_IF(lc_iter->cmd == lc_targ);
        lc_iter = (struct load_command*)((size_t)lc_iter + lc_iter->cmdsize);
    }

    goto fail;
finish:
    result = 0;
    if (lc_res != 0)
    {
        *lc_res = lc_iter;
    }
fail:
    return result;
}

int getpidbyname(const char* proc_name, pid_t* pid_out)
{
    int result = -1;
    struct kinfo_proc *proc_list = NULL;
    size_t length = 0;
    int proc_count = 0;
    int proc_iter = 0;
    const char* proc_name_iter = 0;
    
    static const int name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };

    // Call sysctl with a NULL buffer to get proper length
    SAFE_BAIL(sysctl((int *)name, (sizeof(name) / sizeof(*name)) - 1, NULL, &length, NULL, 0) != 0);

    // Allocate buffer
    proc_list = malloc(length);
    SAFE_BAIL(proc_list == 0);

    // Get the actual process list
    SAFE_BAIL(sysctl((int *)name, (sizeof(name) / sizeof(*name)) - 1, proc_list, &length, NULL, 0) != 0);

    proc_count = length / sizeof(struct kinfo_proc);

    // use getpwuid_r() if you want to be thread-safe

    for (proc_iter = 0; proc_iter < proc_count; proc_iter++)
    {
        proc_name_iter = proc_list[proc_iter].kp_proc.p_comm;
        if (strcmp(proc_name_iter, proc_name) == 0)
        {
            goto finish;
        }
    }
    goto fail;
finish:
    result = 0;
    if (pid_out != 0)
    {
        *pid_out = proc_list[proc_iter].kp_proc.p_pid;
    }
fail:
    SAFE_FREE(proc_list);
    return result;
}

int section_with_sym(struct mach_header_64* mach_header_tmp, size_t sym_address, struct section_64** section_64_out)
{
    int result = -1;
    int cmd_index = 0;
    int sec_index = 0;
    struct load_command* lc_iter = (struct load_command*)&mach_header_tmp[1];
    struct segment_command_64* lc_seg_tmp = 0;
    struct section_64* lc_sec_tmp = 0;

    for (; cmd_index < mach_header_tmp->ncmds; cmd_index++)
    {
        if (lc_iter->cmd == LC_SEGMENT_64)
        {
            lc_seg_tmp = (struct segment_command_64*)lc_iter;
            lc_sec_tmp = (struct section_64*)&lc_seg_tmp[1];
            for(sec_index = 0; sec_index < lc_seg_tmp->nsects; sec_index++)
            {
                FINISH_IF(REGION_CONTAINS(lc_sec_tmp[sec_index].addr, lc_sec_tmp[sec_index].size, sym_address));
            }
        }
        lc_iter = (struct load_command*)((size_t)lc_iter + lc_iter->cmdsize);
    }

    goto fail;
finish:
    result = 0;
    if (section_64_out != 0)
    {
        *section_64_out = lc_sec_tmp;
    }
fail:
    return result;
}
