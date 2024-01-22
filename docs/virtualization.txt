BASIC INTRO
----------------------------------------------------------------------------------------------------

Virtualization is technology that allows you to create multiple simulated environments or dedicated
resources from a single, physical hardware system. Software called a <hypervisor> connects directly
to that hardware and allows you to split 1 system into a seperate, distinct, and secure environments
known as <virtual machines> VMs.

+---------+---------+-------------+
| guest 0 | guest 1 |   . . .     |
+---------+---------+-------------+
| HOST(equiped with a hypervisor) |
+---------------------------------+

Things like CPU, memory and storage as a pool of resources that can be relocated.

As for hypervisors:

0) Standalone (Type 1) hypervisor
+---------+---------+-------------+
| guest 0 | guest 1 |   . . .     |
+---------+---------+-------------+
|            Hypervisor           |
+---------------------------------+
|             Hardware            |
+---------------------------------+

For Type 1, the bare-metal hypervisor, each Virtual Machine (VM) contains a guest OS.

Bare-metal virtualization means that the Type 1 hypervisor has direct access to hardware resources,
which results in better performance.

1) Hosted (Type 2) hypervisor
+---------+---------+-------------+
| guest 0 | guest 1 |   . . .     |
+---------+---------+-------------+
|      Host OS (hypervisor)       |
+---------------------------------+
|             Hardware            |
+---------------------------------+

In Type 2, the hosted hypervisor is an extension of the host OS with each subsequent guest OS
contained in a separate VM.

----------------------------------------------------------------------------------------------------

A hypervisor can perform:

* memory management
  stage 2 translation tables set up by the hypervisor tanslate intermediate physical memory
  addresses to physical memory addresses.

* device emulation

* device assignment
  The hypervisor has the option of assigning individual devices to individual guest operating
  systems so that the guest can own and operate the device without requiring hypervisor
  arbitration.

* exception handling

* instruction trapping

* managing virtual exceptions

* interrupt controller management

  Virtual Interrupts: vIRQs, vFIQs, vSErrors. To signal virtual interrupts to EL0/1, a hypervisor
  must set the corresponding routing bit in HCR_EL2.

* scheduling 

* context switching

* memory translation

                      +-----------------+
  Non-secure state -> | virtual address |
                      +-----------------+
                               | TTBR0/1_EL1
                      +-----------------+
                      |   intermediate  |
                      | physical address|
                      +-----------------+
                               | VTTBR_EL2
                      +-----------------+
                      | physical address|
                      +-----------------+

  The second stage of translation uses the Virtualization Translation Table Base Registers
  VTTBR_EL2 and VTCR_EL2, controlled by the hypervisor.

                +-----------------+
  hypervisor -> | virtual address |
                +-----------------+
                         | TTBR0/1_EL2
                +-----------------+
                | physical address|
                +-----------------+

  For the virtual address space of the hypervisor, a single stage translation is used, controlled
  by the registers, TTBR0/1_EL2 and TCR_EL2.

  ARMv8-A virtualization also introduces the concept of a Virtual Machine ID (VMID). Each virtual
  machine is assigned a VMID, which is an 8-bit value stored in VTTBR_EL2.
  
  VMID is used to tag translation lookaside buffer (TLB) entries, to identify which VM each entry
  belongs to.

* managing multiple virtual address spaces

----------------------------------------------------------------------------------------------------
