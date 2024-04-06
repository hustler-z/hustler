#ifndef AV_ARCH_VCPU_H
#define AV_ARCH_VCPU_H

#include "regs.h"

struct vcpu
{
    struct context_regs regset;
    SREG64 spsr_el1;
    SREG64 elr_el2;
    SREG64 sp_el1;
    SREG64 pc;
    // floating point register F_context_regs
};

void vcpu_save(struct vcpu* _vcpu, struct context_regs* _regset);
void vcpu_save_current();
void vcpu_restore();


#endif
