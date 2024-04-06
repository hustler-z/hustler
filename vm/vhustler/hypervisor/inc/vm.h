#ifndef AV_virtual_machine_H
#define AV_virtual_machine_H

#include "aarch64/inc/arch_vm.h"

struct virtual_machine* CUR_VM;
struct virtual_machine* NEXT_virtual_machine;


/*
    getters-setters
*/
struct virtual_machine* vm_set_current(struct virtual_machine*);
struct virtual_machine* vm_get_current();

/*
    functions
*/
void vm_print_registers(void);
void vm_start_current(void);
void vm_register(struct virtual_machine*);


#endif
