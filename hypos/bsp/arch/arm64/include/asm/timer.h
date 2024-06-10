/**
 * Hustler's Project
 *
 * File:  timer.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ARCH_TIMER_H
#define _ARCH_TIMER_H
// --------------------------------------------------------------
#define CNTKCTL_PCTEN                 (1 << 0)
#define CNTKCTL_VCTEN                 (1 << 1)
#define CNTKCTL_EVNTEN                (1 << 2)
#define CNTKCTL_EVNTDIR               (1 << 3)

#define CNTHCTL_CNTPCT                (1 << 0)
#define CNTHCTL_EVNTEN                (1 << 2)
#define CNTHCTL_EVNTDIR               (1 << 3)

#define CNTP_CTL_ENABLE               (1 << 0)
#define CNTP_CTL_MASK                 (1 << 1)
#define CNTP_CTL_STATUS               (1 << 2)

#define CNTPS_CTL_ENABLE              (1 << 0)
#define CNTPS_CTL_MASK                (1 << 1)
#define CNTPS_CTL_STATUS              (1 << 2)

#define SCR_ENABLE_SECURE_EL1_ACCESS  (1)
#define SCR_DISABLE_SECURE_EL1_ACCESS (0)

#define CNTV_CTL_ENABLE               (1 << 0)
#define CNTV_CTL_MASK                 (1 << 1)
#define CNTV_CTL_STATUS               (1 << 2)

#define CNTHP_CTL_ENABLE              (1 << 0)
#define CNTHP_CTL_MASK                (1 << 1)
#define CNTHP_CTL_STATUS              (1 << 2)
// --------------------------------------------------------------
#endif /* _ARCH_TIMER_H */
