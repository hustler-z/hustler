#include "hypervisor/vm.h"
#include "aarch64/inc/uart.h"

/*
    vm_control.S > asm_start_vm
*/
extern void asm_start_vm();


void register_vm(struct virtual_machine* _vm)
{
    HYP_LOG_N("virtual_machine regiter start");
    HYP_LOG("virtual_machine id: ");
    uart_print_hex(_vm->virtual_machine_ID);
    SERIAL_NEWLINE;
    HYP_LOG("virtual_machine start addr: ");
    uart_print_hex(_vm->memory_region);
    SERIAL_NEWLINE;
}

struct virtual_machine* set_current_vm(struct virtual_machine* _vm)
{
    CURRENT_virtual_machine = _vm;
    return CUR_VM;
}

struct virtual_machine* get_current_vm()
{
    return CUR_VM;
}

void vm_start_current(void)
{
    asm_start_vm();
}
