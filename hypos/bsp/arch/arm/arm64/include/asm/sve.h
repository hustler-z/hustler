/**
 * Hustler's Project
 *
 * File:  sve.h
 * Date:  2024/07/15
 * Usage:
 */

#ifndef _ASM_SVE_H
#define _ASM_SVE_H
// --------------------------------------------------------------


#define SVE_VL_MAX_BITS            2048U
#define SVE_VL_MULTIPLE_VAL        128U

#ifndef __ASSEMBLY__
#include <bsp/type.h>

register_t compute_max_zcr(void);
extern unsigned int sve_get_hw_vl(void);
extern void sve_save_ctx(u64 *sve_ctx, u64 *pregs, int save_ffr);
extern void sve_load_ctx(u64 const *sve_ctx, u64 const *pregs,
                         int restore_ffr);
#endif /* !__ASSEMBLY__ */

// --------------------------------------------------------------
#endif /* _ASM_SVE_H */
