XVISOR                                        2024/04/20 SAT
------------------------------------------------------------
XVISOR = eXtensible Versatile hypervISOR

Hustler's Project

xvisor porting on radxa-zero3w

------------------------------------------------------------
Xvisor is an open-source GPLv2 Type-1 monolithic (i.e. Pure
Type-1) hypervisor.

$ git clone https://github.com/xvisor/xvisor.git

------------------------------------------------------------
ARM (ARMv8.x) Virtualization

Separate EL2 exception-level for hypervisors with it’s own
<xyz>_EL2 MSRs. The Guest/VM will run in EL1/EL0 exception
levels.

Special ARMv8.1-VHE Virtualization Host Extension for better
performance of Type-2 (hosted) hypervisor. Allows Host kernel
(meant for EL1) to run in EL2 by mapping <xyz>_EL1 MSRs to
<abc>_EL2 MSRs in Host mode.

Virtual interrupts for Guest/VM injected using LR registers
of GICv2/GICv3 with virtualization extension. The hypervisor
will save/restore LR registers and emulate all GIC registers
in software except GIC CPU registers.

Virtual timer events for Guest/VM using ARM generic timers
with virtualization support. The hypervisor will save/restore
virtual timer state and manage virtual timer interrupts.

Virtual inter-processor interrupts for Guest/VM by emulating
ICC_SGI1R_EL1 (virtual GICv3) or GICD_SGIR (virtual GICv2).
The save/restore will be handled as part of LR registers save
/restore.

Special ARMv8.3-NV for supporting nested virtualization on
ARMv8. The hypervisor will trap-n-emulate Guest hypervisor
capabilities. The ARMv8.4-NV further enhances nested
virtualization support.

------------------------------------------------------------




------------------------------------------------------------
