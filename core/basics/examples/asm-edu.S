/* 
 * Arm64 Assembly Code Starter
 * ----------------------------------------------------
 * Modern computers execute instructions sequentially.
 * In other words, the next instruction to be executed
 * is at the memory address immediately following the
 * current instruction.
 * ----------------------------------------------------
 * AArch64 general purpose registers and
 * special registers
 *
 * Parameter Passing and/or Scratch Registers(volatile)
 * X0 ~ X7 (W0 ~ W7)
 *
 * Caller-Saved Scratch Registers(volatile)
 * X8(XR) ~ X16(IP0) X17(IP1) X18(PR)
 * W8 ~ W18
 *
 * Callee-Saved Registers(non-volatile)
 * X19 ~ X29(FP)
 * W19 ~ W29
 *
 * Procedure Link Register(hardware special)
 * X30(LR)
 * W30
 *  |
 *  +- hold the return address for subroutines.
 *
 * Zero Register(hardware special)
 * XZR(X31)
 * WZR(W31)
 *
 * Stack Pointer(hardware special)
 * SP(X31)
 *  |
 *  +- hold the address where the stack ends.
 *
 * Program Counter(hardware special)
 * PC
 *
 * Processor State(hardware special)
 * PSTATE
 *   |
 *   +- contains bits that indicate the status of the
 *      current process, including information about
 *      the results of previous operations.
 *
 * ----------------------------------------------------
 * .type <name>, <type_description>
 * .size <name>, <expression> - set the size associated 
 *                              with the symbol.
 * .macro <macname> <macargs> ...
 * .equ   <symbol>, <expression>
 * .set   <symbol>, <expression>
 * .comm  <symbol>, <length> - similar to "extern" in C.
 * ----------------------------------------------------
 * .section <name>
 *            |
 *          .rodata
 *          .data
 *          .text
 *          .bss - Block Started by Symbol is used for
 *                 data storage areas that should be
 *                 initialized to zero at the beginning
 *                 of program execution.
 * ----------------------------------------------------
 * .byte   <expressions>
 * .2byte  <expressions>
 * .hword  <expressions>
 * .short  <expressions>
 * .4byte  <expressions>
 * .word   <expressions>
 * .long   <expressions>
 * .8byte  <expressions>
 * .quad   <expressions>
 * .ascii  "string"
 * .asciz  "string"
 * .string "string"
 * .float  flonums
 * .single flonums
 * .double flonums
 *
 * .align      abs-expr, abs-expr, abs-expr
 * .balign[lw] abs-expr. abs-expr, abs-expr
 * ----------------------------------------------------
 * .include "file"
 * .if <expression>
 * .else
 * .ifdef  <symbol>
 * .ifndef <symbol>
 * .endif
 * ----------------------------------------------------
 * Logical Operations
 * and - bitwise and
 * bic - bitwise bit clear
 * eor - bitwise exclusive or
 * eon - bitwise exclusive not
 * orr - bitwise or
 * orn - bitwise or not, and
 * mvn - bitwise not
 *
 * Shift Operations
 * asr - arithmetic shift right
 * lsr - logical shift right
 * lsl - logical shift left
 * ror - rotate right
 *
 * mul  - multiply
 * sdiv - signed divide
 * udiv - unsigned divide
 * ----------------------------------------------------
 */

.section .rodata

str: .string "Hello, Assembly\n"

.text

.global func
.type   func, "function"
.p2align 4

func:
    add x0, x0, x1
    ret

.global factorial
.type   factorial, %function
factorial:
    mov x3, x0
loop:
    subs x3, x3, #1
    cmp  x3, #0
    b.eq end
    mul  x0, x0, x3
    b loop
end:
    ret
    .size factorial, (. -factorial)

/*
 * ----------------------------------------------------
 * 1) load/store pair
 *    <op>{<size>} Xt0, Xt1, <addr>
 *    
 *    a) ldp Xt0<-Mem[addr]
 *           Xt1<-Mem[addr + size(Xt0)]
 *    b) stp Xt0->Mem[addr]
 *           Xt1->Mem[addr + size(Xt1)]
 * 2) Form PC-relative address
 *    <op> Xt, <label>
 *   
 *    a) adr  Xt<-Address of label
 *    b) adrp Xt<-Page address of label
 * ----------------------------------------------------
 */

.global info
.type   info, "function"

info:
    stp x29, x30, [sp, #-16]!
    adr x0, str
    bl  printf

    mov w0, 0
    ldp x29, x30, [sp], #16
    ret

    .size info, (. -info)

.global pstate
.type   pstate, %function

pstate:
    mrs x0, NZCV
    ret

/*
 * ----------------------------------------------------
 * mrs Xt<-PSTATE
 * msr PSTATE<-Xt
 *
 * br  pc<-Xn
 * ret pc<-X30 or pc<-Xn
 * 
 * PSTATE:
 * NZCV      - condition flags
 * DAIF      - interrupt bits
 * CurrentEL - current exception level
 * ----------------------------------------------------
 * Address Mode
 * 1) Register Address:
 *    [Xn|sp] - addressing method is used to access the
 *              memory address that is contained in the
 *              register Xn or sp.
 * 2) Signed Immediate Offset:
 *    [Xn|sp, #+/-<imm9>]
 * 3) Unsigned Immediate Offset:
 *    [Xn|sp, #<imm12>]
 * 4) Pre-indexed Immediate Offset:
 *    [Xn|sp, #+/-<imm9>]! - The memory address is
 *              computed by adding the unshifted,
 *              signed 9-bit immediate to the number
 *              stored in Xn or sp.
 * 5) Post-indexed Immediate Offset:
 *    [Xn|sp], #+/-<imm9> - Register Xn or sp is used as
 *              the address of the value to be loaded or
 *              stored.
 * 6) Register Offset:
 *    [Xn|sp, Xm, <option>]
 *    e.g. ldr x3, [x2, x1, lsl #3]
 * 7) Literal:
 *    label
 * 8) Pseudo Load:
 *    =<immediate|symbol>
 * ----------------------------------------------------
 */

.global debug
.type   debug, %function

debug:
    cbz x0, error
    add x0, x0, #6
    ret
error:
    mov x0, -1
    ret

    .size debug, (. -debug)
/*
 * cmp -  compare
 * cmn -  compare negative
 * cbz -  compare and branch on zero
 *        cbz Xn, <label>
 * cbnz - compare and branch on nonzero
 */
