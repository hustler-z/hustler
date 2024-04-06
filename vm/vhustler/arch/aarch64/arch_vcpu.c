
#include "inc/arch_vcpu.h"
#include "inc/utils.h"
#include "../../hypervisor/inc/vm.h"
#include "../../libc/inc/std.h"

void vcpu_save(struct VCPU* _vcpu, struct REGISTER_SET* _regset)
{
    memcpy(&_vcpu->regset, _regset, sizeof(struct REGISTER_SET));
    _vcpu->elr_el2 = __arch_elr_el2_read();
    _vcpu->spsr_el1 = __arch_elr_spsr_el1_read();
};

void vcpu_save_current()
{
    vcpu_save(&CURRENT_VM->vcpu0,&TEMP_REGSET);
};

void vcpu_restore(struct VCPU* _vcpu, struct REGISTER_SET* _regset)
{
    memcpy(&_vcpu->regset, _regset, sizeof(struct REGISTER_SET));
    _vcpu->elr_el2 = __arch_elr_el2_read();
    _vcpu->spsr_el1 = __arch_elr_spsr_el1_read();
};

void vcpu_restore_current()
{
    vcpu_restore(&CURRENT_VM->vcpu0,&TEMP_REGSET);
};
