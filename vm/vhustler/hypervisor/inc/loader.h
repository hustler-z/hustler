/*
    Very simple memory loader for VM
*/

#include <stdint.h>
#include "vm.h"

void load_guest_vm(struct virtual_machine *_vm, long offset);
