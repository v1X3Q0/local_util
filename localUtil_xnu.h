#ifndef LOCALUTIL_XNU_H
#define LOCALUTIL_XNU_H
#include <mach-o/loader.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

int getloadcommandfrommach(struct mach_header_64* mach_header_tmp, uint32_t lc_targ, struct load_command** lc_res);
int getpidbyname(const char* proc_name, pid_t* pid_out);
int section_with_sym(struct mach_header_64* mach_header_tmp, size_t sym_address, struct section_64** section_64_out);
int getsegbynamefromheader_64(struct mach_header_64* mach_header_tmp, const char* name, struct segment_command_64** seg_out);

#ifdef __cplusplus
}
#endif

#endif // LOCALUTIL_XNU_H