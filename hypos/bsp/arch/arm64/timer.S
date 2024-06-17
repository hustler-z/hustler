/**
 * Hustler's Project
 *
 * File:  timer.S
 * Date:  2024/05/23
 * Usage: generic timer APIs
 */

#include <asm/define.h>

// --------------------------------------------------------------

/* Note CNTFRQ_EL0 - Counter-timer Frequency register
 * Frequency of system counter.
 */
FUNC(get_cntfrq_el0)
    mrs  x0, CNTFRQ_EL0
    ret
END(get_cntfrq_el0)

FUNC(set_cntfrq_el0)
    msr  CNTFRQ_EL0, x0
    ret
END(set_cntfrq_el0)

/* Note CNTPCT_EL0 - Counter-timer Physical Count register
 */
FUNC(get_phycnt_el0)
    isb
    mrs  x0, CNTPCT_EL0
    ret
END(get_phycnt_el0)

/* Note CNTVCT_EL0 - Counter-timer Virtual Count register
 */
FUNC(get_virtcnt_el0)
    isb
    mrs  x0, CNTVCT_EL0
    ret
END(get_virtcnt_el0)

/* Note CNTKCTL_EL1
 */
FUNC(get_el1_ctrl)
    mrs  x0, CNTKCTL_EL1
    ret
END(get_el1_ctrl)

FUNC(set_el1_ctrl)
    msr  CNTKCTL_EL1, x0
    ret
END(set_el1_ctrl)

/* Note CNTHCTL_EL2
 */
FUNC(get_el2_ctrl)
    mrs  x0, CNTHCTL_EL2
    ret
END(get_el2_ctrl)

FUNC(set_el2_ctrl)
    msr  CNTHCTL_EL2, x0
    ret
END(set_el2_ctrl)

/* Note CNTP_CVAL_EL0
 */
FUNC(get_ns_el1_phy_compval)
    mrs  x0, CNTP_CVAL_EL0
    ret
END(get_ns_el1_phy_compval)

FUNC(set_ns_el1_phy_compval)
    msr  CNTP_CVAL_EL0, x0
    ret
END(set_ns_el1_phy_compval)

/* Note CNTP_TVAL_EL0
 */
FUNC(get_ns_el1_phy_timer_val)
    mrs  x0, CNTP_TVAL_EL0
    ret
END(get_ns_el1_phy_timer_val)

FUNC(set_ns_el1_phy_timer_val)
    msr  CNTP_TVAL_EL0, x0
    ret
END(set_ns_el1_phy_timer_val)

/* Note CNTP_CTL_EL0
 */
FUNC(get_ns_el1_phy_timer_ctrl)
    mrs  x0, CNTP_CTL_EL0
    ret
END(get_ns_el1_phy_timer_ctrl)

FUNC(set_ns_el1_phy_timer_ctrl)
    msr  CNTP_CTL_EL0, x0
    isb
    ret
END(set_ns_el1_phy_timer_ctrl)

/* Note CNTPS_CVAL_EL1
 */
FUNC(get_s_el1_phy_compval)
    mrs  x0, CNTPS_CVAL_EL1
    ret
END(get_s_el1_phy_compval)

FUNC(set_s_el1_phy_compval)
    msr  CNTPS_CVAL_EL1, x0
    ret
END(set_s_el1_phy_compval)

/* Note CNTPS_TVAL_EL1
 */
FUNC(get_s_el1_phy_timer_val)
    mrs  x0, CNTPS_TVAL_EL1
    ret
END(get_s_el1_phy_timer_val)

FUNC(set_s_el1_phy_timer_val)
    msr  CNTPS_TVAL_EL1, x0
    ret
END(set_s_el1_phy_timer_val)

/* Note CNTPS_CTL_EL1
 */
FUNC(get_s_el1_phy_timer_ctrl)
    mrs  x0, CNTPS_CTL_EL1
    ret
END(get_s_el1_phy_timer_ctrl)

FUNC(set_s_el1_phy_timer_ctrl)
    msr  CNTPS_CTL_EL1, x0
    isb
    ret
END(set_s_el1_phy_timer_ctrl)

FUNC(config_secure_el1_timer_access)
    mrs  x1, SCR_EL3
    bfi  x1, x0, #11, #1
    msr  SCR_EL3, x1
    ret
END(config_secure_el1_timer_access)

/* Note CNTV_CVAL_EL0
 */
FUNC(get_el1_virt_compval)
    mrs  x0, CNTV_CVAL_EL0
    ret
END(get_el1_virt_compval)

FUNC(set_el1_virt_compval)
    msr  CNTV_CVAL_EL0, x0
    ret
END(set_el1_virt_compval)

/* Note CNTV_TVAL_EL0
 */
FUNC(get_el1_virt_timer_val)
    mrs  x0, CNTV_TVAL_EL0
    ret
END(get_el1_virt_timer_val)

FUNC(set_el1_virt_timer_val)
    msr  CNTV_TVAL_EL0, x0
    ret
END(set_el1_virt_timer_val)

/* Note CNTV_CTL_EL0
 */
FUNC(get_el1_virt_timer_ctrl)
    mrs  x0, CNTV_CTL_EL0
    ret
END(get_el1_virt_timer_ctrl)

FUNC(set_el1_virt_timer_ctrl)
    msr  CNTV_CTL_EL0, x0
    isb
    ret
END(set_el1_virt_timer_ctrl)

/* Note CNTVOFF_EL2
 */
FUNC(get_virt_counter_offset)
    mrs  x0, CNTVOFF_EL2
    ret
END(get_virt_counter_offset)

FUNC(set_virt_counter_offset)
    msr  CNTVOFF_EL2, x0
    ret
END(set_virt_counter_offset)

/* Note CNTHP_CVAL_EL2
 */
FUNC(get_el2_phy_compval)
    mrs  x0, CNTHP_CVAL_EL2
    ret
END(get_el2_phy_compval)

FUNC(set_el2_phy_compval)
    msr  CNTHP_CVAL_EL2, x0
    ret
END(set_el2_phy_compval)

/* Note CNTHP_TVAL_EL2
 */
FUNC(get_el2_phy_timer_val)
    mrs  x0, CNTHP_TVAL_EL2
    ret
END(get_el2_phy_timer_val)

FUNC(set_el2_phy_timer_val)
    msr  CNTHP_TVAL_EL2, x0
    ret
END(set_el2_phy_timer_val)

/* Note CNTHP_CTL_EL2
 */
FUNC(get_el2_phy_timer_ctrl)
    mrs  x0, CNTHP_CTL_EL2
    ret
END(get_el2_phy_timer_ctrl)

FUNC(set_el2_phy_timer_ctrl)
    msr  CNTHP_CTL_EL2, x0
    isb
    ret
END(set_el2_phy_timer_ctrl)

// --------------------------------------------------------------
