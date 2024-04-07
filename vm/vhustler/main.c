/**
 * vhustler entry
 **/

#include "peripherals/inc/uart.h"
#include "arch/aarch64/inc/arch_vm.h"
#include "hypervisor/inc/loader.h"
#include "hypervisor/inc/hypervisor.h"
#include "hypervisor/inc/vm.h"

extern struct virtual_machine* CUR_VM;
extern struct virtual_machine* NEXT_VM;

struct virtual_machine vm1;
struct vcpu vcpu1;

int main()
{
    hypervisor_init();
    hypervisor_set_interrupts();

    vm1.vm_id = 1;
    vm1.vcpu0.regset.x18 = 0xfeed;
    vm1.vcpu0.regset.x2 = 0xdead;
    vm1.vcpu0.regset.x7 = 0xbeef;
    vm1.vcpu0.regset.x15 = 12;
    vm1.memory_region = 0x60000000;
    load_guest_vm((void *) &vm1, vm1.memory_region);
    // vm1.vcpu0.elr_el2 = vm1.entry_point;
    vm1.vcpu0.spsr_el1 = 0x5;

    register_vm(&vm1);

    CUR_VM = &vm1;

    hypervisor_enable_interrupts();
    vm_start_current();

    while (1) {
        __arch_wfi();
    }

    return 0;
}
