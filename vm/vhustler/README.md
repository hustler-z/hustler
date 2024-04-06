-------------------------------------------------------
Based on armvisor - https://github.com/m8/armvisor.git
-------------------------------------------------------

ARCHITECTURE:
.
├── arch
│   └── aarch64
│       ├── arch_gic.c
│       ├── arch_vcpu.c
│       ├── arch_vm.c
│       ├── exception.S
│       ├── generic_timer.c
│       ├── gicv3
│       │   ├── gicv3_basic.c
│       │   ├── gicv3_basic.h
│       │   ├── gicv3_cpuif.S
│       │   └── gicv3_registers.h
│       ├── handler.c
│       ├── inc
│       │   ├── arch_gic.h
│       │   ├── arch_vcpu.h
│       │   ├── arch_vm.h
│       │   ├── board.h
│       │   ├── generic_timer.h
│       │   ├── irq.h
│       │   ├── mm.h
│       │   ├── ptable.h
│       │   ├── regs.h
│       │   ├── time_asm.h
│       │   └── utils.h
│       ├── irq.c
│       ├── Makefile
│       ├── ptable.c
│       ├── stack_guard.c
│       ├── start.S
│       ├── transition.S
│       └── vm_control.S
├── config.h
├── cook.sh
├── GPATH
├── GRTAGS
├── GTAGS
├── guest-vm
│   ├── guest_vm_data_backup.h
│   └── guest_vm_data.h
├── hypervisor
│   ├── hypervisor.c
│   ├── inc
│   │   ├── hypervisor.h
│   │   ├── loader.h
│   │   ├── mm.h
│   │   ├── scheduler.h
│   │   ├── vcpu.h
│   │   └── vm.h
│   ├── loader.c
│   ├── Makefile
│   ├── mm.c
│   ├── scheduler.c
│   ├── vcpu.c
│   └── vm.c
├── io
│   ├── inc
│   │   └── uart.h
│   └── uart.c
├── libc
│   ├── inc
│   │   ├── std.h
│   │   └── string.h
│   ├── Makefile
│   ├── std.c
│   └── string.c
├── main.c
├── Makefile
├── README.md
├── tags
└── vhustler.ld

-------------------------------------------------------
- DEBUG -

qemu-system-aarch64 -s -S -M virt,virtualization=on \
    -machine gic-version=3 -cpu cortex-a57 \
    -m 2G -nographic -kernel [ *.elf ]
