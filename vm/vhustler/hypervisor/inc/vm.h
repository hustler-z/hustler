#ifndef AV_virtual_machine_H
#define AV_virtual_machine_H

#include "../../arch/aarch64/inc/arch_vm.h"




/**
 * getters-setters
 */
struct virtual_machine* vm_set_current(struct virtual_machine*);
struct virtual_machine* vm_get_current();

/**
 * functions
 */
void vm_print_registers(void);
void vm_start_current(void);
void register_vm(struct virtual_machine*);

#endif
