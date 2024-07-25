/**
 * Hustler's Project
 *
 * File:  sve.c
 * Date:  2024/07/12
 * Usage:
 */

#include <asm/sve.h>
#include <asm/barrier.h>
#include <asm/sysregs.h>
#include <bsp/panic.h>

// --------------------------------------------------------------
static inline register_t vl_to_zcr(unsigned int vl)
{
    ASSERT(vl > 0);
    return ((vl / SVE_VL_MULTIPLE_VAL) - 1U) & ZCR_ELx_LEN_MASK;
}

register_t get_default_cptr_flags(void)
{
    /*
     * Trap all coprocessor registers (0-13) except cp10 and
     * cp11 for VFP.
     *
     * On ARM64 the TCPx bits which we set here (0..9,12,13) are all
     * RES1, i.e. they would trap whether we did this write or not.
     */
    return ((HCPTR_CP_MASK & ~(HCPTR_CP(10) | HCPTR_CP(11))) |
            HCPTR_TTA | HCPTR_TAM);
}

register_t compute_max_zcr(void)
{
    register_t cptr_bits = get_default_cptr_flags();
    register_t zcr = vl_to_zcr(SVE_VL_MAX_BITS);
    unsigned int hw_vl;

    /* Remove trap for SVE resources */
    WRITE_SYSREG(cptr_bits & ~HCPTR_CP(8), CPTR_EL2);
    isb();

    WRITE_SYSREG(zcr, ZCR_EL2);

    hw_vl = sve_get_hw_vl() * 8U;

    /* Restore CPTR_EL2 */
    WRITE_SYSREG(cptr_bits, CPTR_EL2);
    isb();

    return vl_to_zcr(hw_vl);
}
// --------------------------------------------------------------
