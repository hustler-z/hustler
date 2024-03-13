+------------------------------------------------------------------------------+
| ARM64 ASSEMBLY BASICS                                                        |
+------------------------------------------------------------------------------+

SMCCC (SMC Calling Convention) [+] arch/arm64/kernel/smccc-call.S

The Function Identifier is passed on W0 on every SMC and HVC call. The bit
W0[31] determines if the call is Fast (W0[31]==1) or Yielding (W0[31]==0).

In the Fast Call case (W0[31]==1), the bits W0[30:0] determine:
• The service to be invoked
• The function to be invoked
• The calling convention (32-bit or 64-bit) that is in use

Register use in AArch64 SMC and HVC calls

--------------------------------------------------------------------------------
sp_elx  - ELx stack pointer
x30     - The link register
x29     - The frame register
x19~x28 - Registers that are saved by the called function
x18     - The platform register
x17     - Parameter register/The second intra-procedure-call scratch register
x16     - Parameter register/The first intra-procedure-call scratch register
x9~x15  - Parameter registers/Temporary registers
x8      - Parameter register/Indirect result location register

--------------------------------------------------------------------------------
SMC32/HVC32  SMC64/HVC64
w7           x7         Parameter register
                        Optional Client ID in bits[15:0] (ignored for HVC calls)
                        Optional Secure OS ID in bits[31:16]
w6           x6         Parameter register
                        Optional Session ID register
w4~w5        x4~x5      Parameter registers

--------------------------------------------------------------------------------
w1~w3        x1~x3      Parameter registers         SMC and HVC result registers
w0           x0         Function Identifier

--------------------------------------------------------------------------------

Note: The 32-bit W register forms the lower half of the corresponding 64-bit X
      register.

Software Delegated Exception Interface (SDEI) provides a mechanism for
registering and servicing system events from system firmware.

System events are high priority events, which must be serviced immediately by
an OS or hypervisor.

Software Delegated Exception is a software agreement between higher and lower
Exception levels for delegating events from the higher Exception level to the
lower Exception level.

The higher Exception level software is called the dispatcher. The dispatcher
handles the request from lower Exception level and delegates the event.

The lower Exception level software is called the client. The client uses the
interface provided by the dispatcher and handles the events.

sdei_mask_local_cpu()
         |
         +- invoke_sdei_fn(SDEI_1_0_FN_SDEI_PE_MASK, 0, 0, 0, 0, 0, NULL)
                   :
                   +- sdei_firmware_call() [+] drivers/firmware/arm_sdei.c
                               :
               *---------------*---------------*
               |                               |
     a) sdei_smccc_hvc()           b) sdei_smcc_smc()
                                        :
                                        +- smc #0 => cause an exception to EL3

sdei_smccc_hvc()
       |
       +- arm_smccc_hvc()
                |
                +- __arm_smccc_hvc() [+] arch/arm64/kernel/smccc-call.S
                           :
                           +- hvc #0 => cause an exception to EL2

--------------------------------------------------------------------------------
- Cache Maintenance -

<cache> <operation>{, <Xt>}

IC <ic_op>{, <Xt>}
DC <dc_op>{, <Xt>}

--------------------------------------------------------------------------------
- Conditional Branch Instructions -

CBZ Rt, label          => Compare and branch if zero
CBNZ Rt, label         => Compare and branch if not zero
TBZ Rt, bit, label     => Test and branch if Rt<bit> zero
TBNZ Rt, bit, label    => Test and branch if Rt<bit> is not zero

--------------------------------------------------------------------------------

YIELD
Hint that the current thread is performing a task that can be swapped out.

yield() => yield the current processor to other threads

--------------------------------------------------------------------------------
- Basic Data Types -

*---------------------------------------------------------------------------*
| Type            A32       A64        Description                          |
*---------------------------------------------------------------------------*
| int/long        32-bit    32-bit     integer                              |
| short           16-bit    16-bit     integer                              |
| char            8-bit     8-bit      byte                                 |
| long long       64-bit    64-bit     integer                              |
| float           32-bit    32-bit     single-precision IEEE floating-point |
| double          64-bit    64-bit     double-precision IEEE floating-point |
| bool            8-bit     8-bit      Boolean                              |
| void* pointer   32-bit    64-bit     addresses to data or code            |
*---------------------------------------------------------------------------*

Byte                8 bits
Halfword            16 bits
Word                32 bits
Doubleword          64 bits
Quadword            128 bits

--------------------------------------------------------------------------------
The Multi-Processor Affinity Register (MPIDR_EL1) enables software to determine
on which core it is executing, both within a cluster and in a system with
multiple clusters, where it determines on which core and in which cluster it is
executing.

--------------------------------------------------------------------------------
Symmetric Multi-Processing (SMP) is a software architecture that dynamically
determines the roles of individual cores. Each core in the cluster has the same
view of memory and of shared hardware.

--------------------------------------------------------------------------------
- KERNEL INLINE ASSEMBLY -

__asm__ __volatile__ ();

OR
        *------------------▶ it tells compiler that the assembly has an effect,
        |                    so that can avoid being optimized out.
        :
asm [volatile] (
        "<assembly string>"
        [ : <output operands> ---------▶ [<name>] "<constraint>" (<value>)
        [ : <intput operands>
        [ : <clobbers>  ] ] ]
);              |
                :
                ▼
        To prevent the compiler from using a register for a template
        string in an inline assembly string, add the register to the
        clobber list.
        also (a) "memory" => tell the compiler that the assembly code
                             might modify any memory.
             (b) "cc"     => tell the compiler that the assembly code
                             might modify any of the condition flags,
                             in NZCV (aarch64) / CPSR (aarch32).

--------------------------------------------------------------------------------
