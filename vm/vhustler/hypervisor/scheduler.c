#include "inc/vm.h"
#include "../arch/aarch64/inc/ptable.h"

struct virtual_machine* CUR_VM;
struct virtual_machine* NEXT_VM;

extern void asm_start_vm();

void hypervisor_sched_setup()
{
    if (CUR_VM->vm_id == 1) {
        setup_vm1_stage2_level1();
    } else {
        setup_vm2_stage2_level1();
    }

    asm_start_vm();
}

void schedule_next_vm()
{
    struct virtual_machine* buf = CUR_VM;
    CUR_VM = NEXT_VM;
    NEXT_VM = buf;
}
