/**
 * Hustler's Project
 *
 * File:  vcpu.c
 * Date:  2024/05/22
 * Usage:
 */

#include <core/vcpu.h>
#include <core/sched.h>
#include <asm-generic/section.h>

// --------------------------------------------------------------


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

struct vcpu *hypos_vcpus[NR_CPUS] __read_mostly;





// --------------------------------------------------------------
