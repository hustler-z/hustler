
#include "inc/arch_vcpu.h"
#include "inc/utils.h"
#include "../../hypervisor/inc/vm.h"
#include "../../libc/inc/string.h"

extern struct virtual_machine* CUR_VM;
extern struct virtual_machine* NEXT_VM;

struct context_regs TEMP_REGSET;

void vcpu_save(struct vcpu* _vcpu, struct context_regs* _regset)
{
    memcpy(&_vcpu->regset, _regset, sizeof(struct context_regs));
    _vcpu->elr_el2 = __arch_elr_el2_read();
    _vcpu->spsr_el1 = __arch_elr_spsr_el1_read();
};

void vcpu_save_current()
{
    vcpu_save(&CUR_VM->vcpu0, &TEMP_REGSET);
};

void vcpu_restore(struct vcpu* _vcpu, struct context_regs* _regset)
{
    memcpy(&_vcpu->regset, _regset, sizeof(struct context_regs));
    _vcpu->elr_el2 = __arch_elr_el2_read();
    _vcpu->spsr_el1 = __arch_elr_spsr_el1_read();
};

void vcpu_restore_current()
{
    vcpu_restore(&CUR_VM->vcpu0, &TEMP_REGSET);
};
