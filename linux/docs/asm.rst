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
