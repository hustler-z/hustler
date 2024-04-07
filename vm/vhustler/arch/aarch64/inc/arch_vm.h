#ifndef ARCH_VM_H
#define ARCH_VM_H

#include "arch_vcpu.h"

struct virtual_machine
{
    // page table entry point
    struct vcpu vcpu0;
    uint32_t vm_id;
    uint64_t memory_region;
    uint64_t entry_point;
};

#endif
