#include "inc/arch_vm.h"
#include "inc/arch_vcpu.h"
#include "inc/regs.h"
#include "../../hypervisor/inc/vcpu.h"

void vm_save(struct VM* _vm)
{
    // Should be changed with all cores
    vcpu_save(&_vm->vcpu0, &TEMP_REGSET);

}
