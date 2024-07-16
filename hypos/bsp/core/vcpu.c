/**
 * Hustler's Project
 *
 * File:  vcpu.c
 * Date:  2024/05/22
 * Usage:
 */

#include <org/vcpu.h>
#include <bsp/sched.h>
#include <org/section.h>

// --------------------------------------------------------------
DEFINE_PERCPU(struct vcpu *, current_vcpu);

/* --------------------------------------------------------------
 *
 * ID_AA64MMFR0_EL1
 * [31,28] Indicates support for 4KB memory translation
 *         granule size.
 * [43,40] Indicates support for 4KB memory granule size
 *         at stage 2.
 *
 *     +------------------------------+
 *     |            vCPU              <---+
 * +<---  mrs x0, ID_AA64MMFR0_EL1    |   |
 * |   +------------------------------+   |
 * |  ----------------------------------  | ERET
 * |   +------------------------------+   |
 * |   |                              |   |
 * +--->  ESR_EL2                     |   |
 *     |                              --->+
 *     |  Trap Handler in Hypervisor  |
 *     +------------------------------+
 *
 * --------------------------------------------------------------
 */






// --------------------------------------------------------------
