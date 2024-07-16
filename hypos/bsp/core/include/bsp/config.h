/**
 * Hustler's Project
 *
 * File:  config.h
 * Date:  2024/07/10
 * Usage: set up hypos configuration
 */

#ifndef _BSP_CONFIG_H
#define _BSP_CONFIG_H
// --------------------------------------------------------------

#define IS_ENABLED(cfg)           (!!(cfg))

#define CFG_ARM_CORTEX_A73        0
#define CFG_WORDS_BIGENDIAN       0
#define CFG_GIC_DEBUG             0

// --------------------------------------------------------------

/* Implementation Configuration
 */
#define IS_IMPLEMENTED(imp)        (!!(imp))

#define __IRQ_IMPL                 1
#define __TIME_IMPL                1
#define __TIMER_IMPL               1
#define __NOTIFIER_CHAIN_IMPL      1
#define __GIC_IMPL                 1
#define __VGIC_IMPL                0
#define __SOFTIRQ_IMPL             1
#define __COMPLEX_SPINLOCK_IMPL    1
#define __RWLOCK_IMPL              1
#define __VSYSREG_IMPL             0
#define __TASKLET_IMPL             1
#define __RCU_IMPL                 1
#define __PSCI_IMPL                1
#define __SCHED_IMPL               0
#define __IOMEM_IMPL               0
#define __VGIC_V3_IMPL             0
#define __VGIC_IMPL                0
#define __ROUTE_IRQ_TO_GUEST       0
#define __WAIT_QUEUE_IMPL          0

// --------------------------------------------------------------
#define IS_SUPPORTED(sup)          (!!(sup))

#define SUP__ID_AA64ZFR0_EL1       0
#define SUP__ID_PFR2_EL1           0
// --------------------------------------------------------------
#endif /* _BSP_CONFIG_H */
