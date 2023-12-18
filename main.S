/* 
 * Arm64 Assembly Code Starter
 * ----------------------------------------------------
 * .type <name>, <type_description>
 * .size <name>, <expression> - set the size associated 
 *                              with the symbol.
 * .macro <macname> <macargs> ...
 * .equ   <name>, <expression>
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
 *
 * cmp  - compare
 * tst  - test bits
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

/*
 * ----------------------------------------------------
 * Frame pointer           - x29
 * PSTATE register
 * Procedure link register - x30
 * Stack pointer           - sp
 * Zero pointer            - xzr/wzr
 * Program counter         - pc
 * ----------------------------------------------------
 */

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
 * PSTATE:
 * NZCV      - condition flags
 * DAIF      - interrupt bits
 * CurrentEL - current exception level
 * ----------------------------------------------------
 */


