A64 INSTRUCTION SET
----------------------------------------------------------------------------------------

ABS: Absolute value.

ADC: Add with Carry.

ADCS: Add with Carry, setting flags.

ADD (extended register): Add (extended register).

ADD (immediate): Add (immediate).

ADD (shifted register): Add (shifted register).

ADDG: Add with Tag.

ADDPT: Add checked pointer.

ADDS (extended register): Add (extended register), setting flags.

ADDS (immediate): Add (immediate), setting flags.

ADDS (shifted register): Add (shifted register), setting flags.

ADR: Form PC-relative address.

ADRP: Form PC-relative address to 4KB page.

AND (immediate): Bitwise AND (immediate).

AND (shifted register): Bitwise AND (shifted register).

ANDS (immediate): Bitwise AND (immediate), setting flags.

ANDS (shifted register): Bitwise AND (shifted register), setting flags.

ASR (immediate): Arithmetic Shift Right (immediate): an alias of SBFM.

ASR (register): Arithmetic Shift Right (register): an alias of ASRV.

ASRV: Arithmetic Shift Right Variable.

AT: Address Translate: an alias of SYS.

AUTDA, AUTDZA: Authenticate Data address, using key A.

AUTDB, AUTDZB: Authenticate Data address, using key B.

AUTIA, AUTIA1716, AUTIASP, AUTIAZ, AUTIZA: Authenticate Instruction address, using key A.

AUTIASPPC (immediate): Authenticate return address, using key A (immediate).

AUTIASPPC (register): Authenticate return address, using key A (register).

AUTIB, AUTIB1716, AUTIBSP, AUTIBZ, AUTIZB: Authenticate Instruction address, using key B.

AUTIBSPPC (immediate): Authenticate return address, using key B (immediate).

AUTIBSPPC (register): Authenticate return address, using key B (register).

AXFLAG: Convert floating-point condition flags from Arm to external format.

B: Branch.

B.cond: Branch conditionally.

BC.cond: Branch Consistent conditionally.

BFC: Bitfield Clear: an alias of BFM.

BFI: Bitfield Insert: an alias of BFM.

BFM: Bitfield Move.

BFXIL: Bitfield extract and insert at low end: an alias of BFM.

BIC (shifted register): Bitwise Bit Clear (shifted register).

BICS (shifted register): Bitwise Bit Clear (shifted register), setting flags.

BL: Branch with Link.

BLR: Branch with Link to Register.

BLRAA, BLRAAZ, BLRAB, BLRABZ: Branch with Link to Register, with pointer authentication.

BR: Branch to Register.

BRAA, BRAAZ, BRAB, BRABZ: Branch to Register, with pointer authentication.

BRB: Branch Record Buffer: an alias of SYS.

BRK: Breakpoint instruction.

BTI: Branch Target Identification.

CAS, CASA, CASAL, CASL: Compare and Swap word or doubleword in memory.

CASB, CASAB, CASALB, CASLB: Compare and Swap byte in memory.

CASH, CASAH, CASALH, CASLH: Compare and Swap halfword in memory.

CASP, CASPA, CASPAL, CASPL: Compare and Swap Pair of words or doublewords in memory.

CBNZ: Compare and Branch on Nonzero.

CBZ: Compare and Branch on Zero.

CCMN (immediate): Conditional Compare Negative (immediate).

CCMN (register): Conditional Compare Negative (register).

CCMP (immediate): Conditional Compare (immediate).

CCMP (register): Conditional Compare (register).

CFINV: Invert Carry Flag.

CFP: Control Flow Prediction Restriction by Context: an alias of SYS.

CHKFEAT: Check feature status.

CINC: Conditional Increment: an alias of CSINC.

CINV: Conditional Invert: an alias of CSINV.

CLRBHB: Clear Branch History.

CLREX: Clear Exclusive.

CLS: Count Leading Sign bits.

CLZ: Count Leading Zeros.

CMN (extended register): Compare Negative (extended register): an alias of ADDS
                         (extended register).

CMN (immediate): Compare Negative (immediate): an alias of ADDS (immediate).

CMN (shifted register): Compare Negative (shifted register): an alias of ADDS
                        (shifted register).

CMP (extended register): Compare (extended register): an alias of SUBS (extended register).

CMP (immediate): Compare (immediate): an alias of SUBS (immediate).

CMP (shifted register): Compare (shifted register): an alias of SUBS (shifted register).

CMPP: Compare with Tag: an alias of SUBPS.

CNEG: Conditional Negate: an alias of CSNEG.

CNT: Count bits.

COSP: Clear Other Speculative Prediction Restriction by Context: an alias of SYS.

CPP: Cache Prefetch Prediction Restriction by Context: an alias of SYS.

CPYFP, CPYFM, CPYFE: Memory Copy Forward-only.

CPYFPN, CPYFMN, CPYFEN: Memory Copy Forward-only, reads and writes non-temporal.

CPYFPRN, CPYFMRN, CPYFERN: Memory Copy Forward-only, reads non-temporal.

CPYFPRT, CPYFMRT, CPYFERT: Memory Copy Forward-only, reads unprivileged.

CPYFPRTN, CPYFMRTN, CPYFERTN: Memory Copy Forward-only, reads unprivileged,
                              reads and writes non-temporal.

CPYFPRTRN, CPYFMRTRN, CPYFERTRN: Memory Copy Forward-only, reads unprivileged
                                 and non-temporal.

CPYFPRTWN, CPYFMRTWN, CPYFERTWN: Memory Copy Forward-only, reads unprivileged,
                                 writes non-temporal.

CPYFPT, CPYFMT, CPYFET: Memory Copy Forward-only, reads and writes unprivileged.

CPYFPTN, CPYFMTN, CPYFETN: Memory Copy Forward-only, reads and writes unprivileged
                           and non-temporal.

CPYFPTRN, CPYFMTRN, CPYFETRN: Memory Copy Forward-only, reads and writes unprivileged,
                              reads non-temporal.

CPYFPTWN, CPYFMTWN, CPYFETWN: Memory Copy Forward-only, reads and writes unprivileged,
                              writes nontemporal.

CPYFPWN, CPYFMWN, CPYFEWN: Memory Copy Forward-only, writes non-temporal.

CPYFPWT, CPYFMWT, CPYFEWT: Memory Copy Forward-only, writes unprivileged.

CPYFPWTN, CPYFMWTN, CPYFEWTN: Memory Copy Forward-only, writes unprivileged,
                              reads and writes nontemporal.

CPYFPWTRN, CPYFMWTRN, CPYFEWTRN: Memory Copy Forward-only, writes unprivileged,
                                 reads non-temporal.

CPYFPWTWN, CPYFMWTWN, CPYFEWTWN: Memory Copy Forward-only, writes unprivileged
                                 and non-temporal.

CPYP, CPYM, CPYE: Memory Copy.

CPYPN, CPYMN, CPYEN: Memory Copy, reads and writes non-temporal.

CPYPRN, CPYMRN, CPYERN: Memory Copy, reads non-temporal.

CPYPRT, CPYMRT, CPYERT: Memory Copy, reads unprivileged.

CPYPRTN, CPYMRTN, CPYERTN: Memory Copy, reads unprivileged, reads and writes non-temporal.

CPYPRTRN, CPYMRTRN, CPYERTRN: Memory Copy, reads unprivileged and non-temporal.

CPYPRTWN, CPYMRTWN, CPYERTWN: Memory Copy, reads unprivileged, writes non-temporal.

CPYPT, CPYMT, CPYET: Memory Copy, reads and writes unprivileged.

CPYPTN, CPYMTN, CPYETN: Memory Copy, reads and writes unprivileged and non-temporal.

CPYPTRN, CPYMTRN, CPYETRN: Memory Copy, reads and writes unprivileged, reads non-temporal.

CPYPTWN, CPYMTWN, CPYETWN: Memory Copy, reads and writes unprivileged, writes non-temporal.

CPYPWN, CPYMWN, CPYEWN: Memory Copy, writes non-temporal.

CPYPWT, CPYMWT, CPYEWT: Memory Copy, writes unprivileged.

CPYPWTN, CPYMWTN, CPYEWTN: Memory Copy, writes unprivileged, reads and writes non-temporal.

CPYPWTRN, CPYMWTRN, CPYEWTRN: Memory Copy, writes unprivileged, reads non-temporal.

CPYPWTWN, CPYMWTWN, CPYEWTWN: Memory Copy, writes unprivileged and non-temporal.

CRC32B, CRC32H, CRC32W, CRC32X: CRC32 checksum.

CRC32CB, CRC32CH, CRC32CW, CRC32CX: CRC32C checksum.

CSDB: Consumption of Speculative Data Barrier.

CSEL: Conditional Select.

CSET: Conditional Set: an alias of CSINC.

CSETM: Conditional Set Mask: an alias of CSINV.

CSINC: Conditional Select Increment.

CSINV: Conditional Select Invert.

CSNEG: Conditional Select Negation.

CTZ: Count Trailing Zeros.

DC: Data Cache operation: an alias of SYS.

DCPS1: Debug Change PE State to EL1.

DCPS2: Debug Change PE State to EL2.

DCPS3: Debug Change PE State to EL3.

DGH: Data Gathering Hint.

DMB: Data Memory Barrier.

DRPS: Debug restore PE state.

DSB: Data Synchronization Barrier.

DVP: Data Value Prediction Restriction by Context: an alias of SYS.

EON (shifted register): Bitwise Exclusive-OR NOT (shifted register).

EOR (immediate): Bitwise Exclusive-OR (immediate).

EOR (shifted register): Bitwise Exclusive-OR (shifted register).

ERET: Exception Return.

ERETAA, ERETAB: Exception Return, with pointer authentication.

ESB: Error Synchronization Barrier.

EXTR: Extract register.

GCSB: Guarded Control Stack Barrier.

GCSPOPCX: Guarded Control Stack Pop and Compare exception return record: an alias of SYS.

GCSPOPM: Guarded Control Stack Pop: an alias of SYSL.

GCSPOPX: Guarded Control Stack Pop exception return record: an alias of SYS.

GCSPUSHM: Guarded Control Stack Push: an alias of SYS.

GCSPUSHX: Guarded Control Stack Push exception return record: an alias of SYS.

GCSSS1: Guarded Control Stack Switch Stack 1: an alias of SYS.

GCSSS2: Guarded Control Stack Switch Stack 2: an alias of SYSL.

GCSSTR: Guarded Control Stack Store.

GCSSTTR: Guarded Control Stack unprivileged Store.

GMI: Tag Mask Insert.

HINT: Hint instruction.

HLT: Halt instruction.

HVC: Hypervisor Call.

IC: Instruction Cache operation: an alias of SYS.

IRG: Insert Random Tag.

ISB: Instruction Synchronization Barrier.

LD64B: Single-copy Atomic 64-byte Load.

LDADD, LDADDA, LDADDAL, LDADDL: Atomic add on word or doubleword in memory.

LDADDB, LDADDAB, LDADDALB, LDADDLB: Atomic add on byte in memory.

LDADDH, LDADDAH, LDADDALH, LDADDLH: Atomic add on halfword in memory.

LDAPR: Load-Acquire RCpc Register.

LDAPRB: Load-Acquire RCpc Register Byte.

LDAPRH: Load-Acquire RCpc Register Halfword.

LDAPUR: Load-Acquire RCpc Register (unscaled).

LDAPURB: Load-Acquire RCpc Register Byte (unscaled).

LDAPURH: Load-Acquire RCpc Register Halfword (unscaled).

LDAPURSB: Load-Acquire RCpc Register Signed Byte (unscaled).

LDAPURSH: Load-Acquire RCpc Register Signed Halfword (unscaled).

LDAPURSW: Load-Acquire RCpc Register Signed Word (unscaled).

LDAR: Load-Acquire Register.

LDARB: Load-Acquire Register Byte.

LDARH: Load-Acquire Register Halfword.

LDAXP: Load-Acquire Exclusive Pair of Registers.

LDAXR: Load-Acquire Exclusive Register.

LDAXRB: Load-Acquire Exclusive Register Byte.

LDAXRH: Load-Acquire Exclusive Register Halfword.

LDCLR, LDCLRA, LDCLRAL, LDCLRL: Atomic bit clear on word or doubleword in memory.

LDCLRB, LDCLRAB, LDCLRALB, LDCLRLB: Atomic bit clear on byte in memory.

LDCLRH, LDCLRAH, LDCLRALH, LDCLRLH: Atomic bit clear on halfword in memory.

LDCLRP, LDCLRPA, LDCLRPAL, LDCLRPL: Atomic bit clear on quadword in memory.

LDEOR, LDEORA, LDEORAL, LDEORL: Atomic Exclusive-OR on word or doubleword in memory.

LDEORB, LDEORAB, LDEORALB, LDEORLB: Atomic Exclusive-OR on byte in memory.

LDEORH, LDEORAH, LDEORALH, LDEORLH: Atomic Exclusive-OR on halfword in memory.

LDG: Load Allocation Tag.

LDGM: Load Tag Multiple.

LDIAPP: Load-Acquire RCpc ordered Pair of registers.

LDLAR: Load LOAcquire Register.

LDLARB: Load LOAcquire Register Byte.

LDLARH: Load LOAcquire Register Halfword.

LDNP: Load Pair of Registers, with non-temporal hint.

LDP: Load Pair of Registers.

LDPSW: Load Pair of Registers Signed Word.

LDR (immediate): Load Register (immediate).

LDR (literal): Load Register (literal).

LDR (register): Load Register (register).

LDRAA, LDRAB: Load Register, with pointer authentication.

LDRB (immediate): Load Register Byte (immediate).

LDRB (register): Load Register Byte (register).

LDRH (immediate): Load Register Halfword (immediate).

LDRH (register): Load Register Halfword (register).

LDRSB (immediate): Load Register Signed Byte (immediate).

LDRSB (register): Load Register Signed Byte (register).

LDRSH (immediate): Load Register Signed Halfword (immediate).

LDRSH (register): Load Register Signed Halfword (register).

LDRSW (immediate): Load Register Signed Word (immediate).

LDRSW (literal): Load Register Signed Word (literal).

LDRSW (register): Load Register Signed Word (register).

LDSET, LDSETA, LDSETAL, LDSETL: Atomic bit set on word or doubleword in memory.

LDSETB, LDSETAB, LDSETALB, LDSETLB: Atomic bit set on byte in memory.

LDSETH, LDSETAH, LDSETALH, LDSETLH: Atomic bit set on halfword in memory.

LDSETP, LDSETPA, LDSETPAL, LDSETPL: Atomic bit set on quadword in memory.

LDSMAX, LDSMAXA, LDSMAXAL, LDSMAXL: Atomic signed maximum on word or doubleword in memory.

LDSMAXB, LDSMAXAB, LDSMAXALB, LDSMAXLB: Atomic signed maximum on byte in memory.

LDSMAXH, LDSMAXAH, LDSMAXALH, LDSMAXLH: Atomic signed maximum on halfword in memory.

LDSMIN, LDSMINA, LDSMINAL, LDSMINL: Atomic signed minimum on word or doubleword in memory.

LDSMINB, LDSMINAB, LDSMINALB, LDSMINLB: Atomic signed minimum on byte in memory.

LDSMINH, LDSMINAH, LDSMINALH, LDSMINLH: Atomic signed minimum on halfword in memory.

LDTR: Load Register (unprivileged).

LDTRB: Load Register Byte (unprivileged).

LDTRH: Load Register Halfword (unprivileged).

LDTRSB: Load Register Signed Byte (unprivileged).

LDTRSH: Load Register Signed Halfword (unprivileged).

LDTRSW: Load Register Signed Word (unprivileged).

LDUMAX, LDUMAXA, LDUMAXAL, LDUMAXL: Atomic unsigned maximum on word or doubleword in memory.

LDUMAXB, LDUMAXAB, LDUMAXALB, LDUMAXLB: Atomic unsigned maximum on byte in memory.

LDUMAXH, LDUMAXAH, LDUMAXALH, LDUMAXLH: Atomic unsigned maximum on halfword in memory.

LDUMIN, LDUMINA, LDUMINAL, LDUMINL: Atomic unsigned minimum on word or doubleword in memory.

LDUMINB, LDUMINAB, LDUMINALB, LDUMINLB: Atomic unsigned minimum on byte in memory.

LDUMINH, LDUMINAH, LDUMINALH, LDUMINLH: Atomic unsigned minimum on halfword in memory.

LDUR: Load Register (unscaled).

LDURB: Load Register Byte (unscaled).

LDURH: Load Register Halfword (unscaled).

LDURSB: Load Register Signed Byte (unscaled).

LDURSH: Load Register Signed Halfword (unscaled).

LDURSW: Load Register Signed Word (unscaled).

LDXP: Load Exclusive Pair of Registers.

LDXR: Load Exclusive Register.

LDXRB: Load Exclusive Register Byte.

LDXRH: Load Exclusive Register Halfword.

LSL (immediate): Logical Shift Left (immediate): an alias of UBFM.

LSL (register): Logical Shift Left (register): an alias of LSLV.

LSLV: Logical Shift Left Variable.

LSR (immediate): Logical Shift Right (immediate): an alias of UBFM.

LSR (register): Logical Shift Right (register): an alias of LSRV.

LSRV: Logical Shift Right Variable.

MADD: Multiply-Add.

MADDPT: Multiply-Add checked pointer.

MNEG: Multiply-Negate: an alias of MSUB.

MOV (bitmask immediate): Move (bitmask immediate): an alias of ORR (immediate).

MOV (inverted wide immediate): Move (inverted wide immediate): an alias of MOVN.

MOV (register): Move (register): an alias of ORR (shifted register).

MOV (to/from SP): Move between register and stack pointer: an alias of ADD (immediate).

MOV (wide immediate): Move (wide immediate): an alias of MOVZ.

MOVK: Move wide with keep.

MOVN: Move wide with NOT.

MOVZ: Move wide with zero.

MRRS: Move System Register to two adjacent general-purpose registers.

MRS: Move System Register to general-purpose register.

MSR (immediate): Move immediate value to Special Register.

MSR (register): Move general-purpose register to System Register.

MSRR: Move two adjacent general-purpose registers to System Register.

MSUB: Multiply-Subtract.

MSUBPT: Multiply-Subtract checked pointer.

MUL: Multiply: an alias of MADD.

MVN: Bitwise NOT: an alias of ORN (shifted register).

NEG (shifted register): Negate (shifted register): an alias of SUB (shifted register).

NEGS: Negate, setting flags: an alias of SUBS (shifted register).

NGC: Negate with Carry: an alias of SBC.

NGCS: Negate with Carry, setting flags: an alias of SBCS.

NOP: No Operation.

ORN (shifted register): Bitwise OR NOT (shifted register).

ORR (immediate): Bitwise OR (immediate).

ORR (shifted register): Bitwise OR (shifted register).

PACDA, PACDZA: Pointer Authentication Code for Data address, using key A.

PACDB, PACDZB: Pointer Authentication Code for Data address, using key B.

PACGA: Pointer Authentication Code, using Generic key.

PACIA, PACIA1716, PACIASP, PACIAZ, PACIZA: Pointer Authentication Code for
                                           Instruction address, using key A.

PACIASPPC: Pointer Authentication Code for return address, using key A.

PACIB, PACIB1716, PACIBSP, PACIBZ, PACIZB: Pointer Authentication Code for
                                           Instruction address, using key B.

PACIBSPPC: Pointer Authentication Code for return address, using key B.

PACM: Pointer authentication modifier.

PACNBIASPPC: Pointer Authentication Code for return address, using key A, not a Branch Target.

PACNBIBSPPC: Pointer Authentication Code for return address, using key B, not a Branch Target.

PRFM (immediate): Prefetch Memory (immediate).

PRFM (literal): Prefetch Memory (literal).

PRFM (register): Prefetch Memory (register).

PRFUM: Prefetch Memory (unscaled offset).

PSB: Profiling Synchronization Barrier.

PSSBB: Physical Speculative Store Bypass Barrier: an alias of DSB.

RBIT: Reverse Bits.

RCWCAS, RCWCASA, RCWCASL, RCWCASAL: Read Check Write Compare and Swap doubleword in memory.

RCWCASP, RCWCASPA, RCWCASPL, RCWCASPAL: Read Check Write Compare and Swap quadword in memory.

RCWCLR, RCWCLRA, RCWCLRL, RCWCLRAL: Read Check Write atomic bit Clear on doubleword in memory.

RCWCLRP, RCWCLRPA, RCWCLRPL, RCWCLRPAL: Read Check Write atomic bit Clear on quadword in memory.

RCWSCAS, RCWSCASA, RCWSCASL, RCWSCASAL: Read Check Write Software Compare and Swap
                                        doubleword in memory.

RCWSCASP, RCWSCASPA, RCWSCASPL, RCWSCASPAL: Read Check Write Software Compare and
                                            Swap quadword in memory.

RCWSCLR, RCWSCLRA, RCWSCLRL, RCWSCLRAL: Read Check Write Software atomic bit Clear
                                        on doubleword in memory.

RCWSCLRP, RCWSCLRPA, RCWSCLRPL, RCWSCLRPAL: Read Check Write Software atomic bit
                                            Clear on quadword in memory.

RCWSET, RCWSETA, RCWSETL, RCWSETAL: Read Check Write atomic bit Set on doubleword in memory.

RCWSETP, RCWSETPA, RCWSETPL, RCWSETPAL: Read Check Write atomic bit Set on quadword in memory.

RCWSSET, RCWSSETA, RCWSSETL, RCWSSETAL: Read Check Write Software atomic bit
                                        Set on doubleword in memory.

RCWSSETP, RCWSSETPA, RCWSSETPL, RCWSSETPAL: Read Check Write Software atomic
                                            bit Set on quadword in memory.

RCWSSWP, RCWSSWPA, RCWSSWPL, RCWSSWPAL: Read Check Write Software Swap doubleword in memory.

RCWSSWPP, RCWSSWPPA, RCWSSWPPL, RCWSSWPPAL: Read Check Write Software Swap quadword in memory.

RCWSWP, RCWSWPA, RCWSWPL, RCWSWPAL: Read Check Write Swap doubleword in memory.

RCWSWPP, RCWSWPPA, RCWSWPPL, RCWSWPPAL: Read Check Write Swap quadword in memory.

RET: Return from subroutine.

RETAA, RETAB: Return from subroutine, with pointer authentication.

RETAASPPC, RETABSPPC (immediate): Return from subroutine, with enhanced pointer
                                  authentication return (immediate).

RETAASPPC, RETABSPPC (register): Return from subroutine, with enhanced pointer
                                 authentication return (register).

REV: Reverse Bytes.

REV16: Reverse bytes in 16-bit halfwords.

REV32: Reverse bytes in 32-bit words.

REV64: Reverse Bytes: an alias of REV.

RMIF: Rotate, Mask Insert Flags.

ROR (immediate): Rotate right (immediate): an alias of EXTR.

ROR (register): Rotate Right (register): an alias of RORV.

RORV: Rotate Right Variable.

RPRFM: Range Prefetch Memory.

SB: Speculation Barrier.

SBC: Subtract with Carry.

SBCS: Subtract with Carry, setting flags.

SBFIZ: Signed Bitfield Insert in Zero: an alias of SBFM.

SBFM: Signed Bitfield Move.

SBFX: Signed Bitfield Extract: an alias of SBFM.

SDIV: Signed Divide.

SETF8, SETF16: Evaluation of 8 or 16 bit flag values.

SETGP, SETGM, SETGE: Memory Set with tag setting.

SETGPN, SETGMN, SETGEN: Memory Set with tag setting, non-temporal.

SETGPT, SETGMT, SETGET: Memory Set with tag setting, unprivileged.

SETGPTN, SETGMTN, SETGETN: Memory Set with tag setting, unprivileged and non-temporal.

SETP, SETM, SETE: Memory Set.

SETPN, SETMN, SETEN: Memory Set, non-temporal.

SETPT, SETMT, SETET: Memory Set, unprivileged.

SETPTN, SETMTN, SETETN: Memory Set, unprivileged and non-temporal.

SEV: Send Event.

SEVL: Send Event Local.

SMADDL: Signed Multiply-Add Long.

SMAX (immediate): Signed Maximum (immediate).

SMAX (register): Signed Maximum (register).

SMC: Secure Monitor Call.

SMIN (immediate): Signed Minimum (immediate).

SMIN (register): Signed Minimum (register).

SMNEGL: Signed Multiply-Negate Long: an alias of SMSUBL.

SMSTART: Enables access to Streaming SVE mode and SME architectural state:
         an alias of MSR (immediate).

SMSTOP: Disables access to Streaming SVE mode and SME architectural state:
        an alias of MSR (immediate).

SMSUBL: Signed Multiply-Subtract Long.

SMULH: Signed Multiply High.

SMULL: Signed Multiply Long: an alias of SMADDL.

SSBB: Speculative Store Bypass Barrier: an alias of DSB.

ST2G: Store Allocation Tags.

ST64B: Single-copy Atomic 64-byte Store without status result.

ST64BV: Single-copy Atomic 64-byte Store with status result.

ST64BV0: Single-copy Atomic 64-byte EL0 Store with status result.

STADD, STADDL: Atomic add on word or doubleword in memory, without return:
               an alias of LDADD, LDADDA, LDADDAL, LDADDL.

STADDB, STADDLB: Atomic add on byte in memory, without return: an alias of
                 LDADDB, LDADDAB, LDADDALB, LDADDLB.

STADDH, STADDLH: Atomic add on halfword in memory, without return: an alias
                 of LDADDH, LDADDAH, LDADDALH, LDADDLH.

STCLR, STCLRL: Atomic bit clear on word or doubleword in memory, without return:
               an alias of LDCLR, LDCLRA, LDCLRAL, LDCLRL.

STCLRB, STCLRLB: Atomic bit clear on byte in memory, without return: an alias
                 of LDCLRB, LDCLRAB, LDCLRALB, LDCLRLB.

STCLRH, STCLRLH: Atomic bit clear on halfword in memory, without return: an
                 alias of LDCLRH, LDCLRAH, LDCLRALH, LDCLRLH.

STEOR, STEORL: Atomic Exclusive-OR on word or doubleword in memory, without
               return: an alias of LDEOR, LDEORA, LDEORAL, LDEORL.

STEORB, STEORLB: Atomic Exclusive-OR on byte in memory, without return: an
                 alias of LDEORB, LDEORAB, LDEORALB, LDEORLB.

STEORH, STEORLH: Atomic Exclusive-OR on halfword in memory, without return:
                 an alias of LDEORH, LDEORAH, LDEORALH, LDEORLH.

STG: Store Allocation Tag.

STGM: Store Tag Multiple.

STGP: Store Allocation Tag and Pair of registers.

STILP: Store-Release ordered Pair of registers.

STLLR: Store LORelease Register.

STLLRB: Store LORelease Register Byte.

STLLRH: Store LORelease Register Halfword.

STLR: Store-Release Register.

STLRB: Store-Release Register Byte.

STLRH: Store-Release Register Halfword.

STLUR: Store-Release Register (unscaled).

STLURB: Store-Release Register Byte (unscaled).

STLURH: Store-Release Register Halfword (unscaled).

STLXP: Store-Release Exclusive Pair of registers.

STLXR: Store-Release Exclusive Register.

STLXRB: Store-Release Exclusive Register Byte.

STLXRH: Store-Release Exclusive Register Halfword.

STNP: Store Pair of Registers, with non-temporal hint.

STP: Store Pair of Registers.

STR (immediate): Store Register (immediate).

STR (register): Store Register (register).

STRB (immediate): Store Register Byte (immediate).

STRB (register): Store Register Byte (register).

STRH (immediate): Store Register Halfword (immediate).

STRH (register): Store Register Halfword (register).

STSET, STSETL: Atomic bit set on word or doubleword in memory, without return:
               an alias of LDSET, LDSETA, LDSETAL, LDSETL.

STSETB, STSETLB: Atomic bit set on byte in memory, without return: an alias of
                 LDSETB, LDSETAB, LDSETALB, LDSETLB.

STSETH, STSETLH: Atomic bit set on halfword in memory, without return: an alias
                 of LDSETH, LDSETAH, LDSETALH, LDSETLH.

STSMAX, STSMAXL: Atomic signed maximum on word or doubleword in memory, without
                 return: an alias of LDSMAX, LDSMAXA, LDSMAXAL, LDSMAXL.

STSMAXB, STSMAXLB: Atomic signed maximum on byte in memory, without return: an
                   alias of LDSMAXB, LDSMAXAB, LDSMAXALB, LDSMAXLB.

STSMAXH, STSMAXLH: Atomic signed maximum on halfword in memory, without return:
                   an alias of LDSMAXH, LDSMAXAH, LDSMAXALH, LDSMAXLH.

STSMIN, STSMINL: Atomic signed minimum on word or doubleword in memory, without
                 return: an alias of LDSMIN, LDSMINA, LDSMINAL, LDSMINL.

STSMINB, STSMINLB: Atomic signed minimum on byte in memory, without return: an
                   alias of LDSMINB, LDSMINAB, LDSMINALB, LDSMINLB.

STSMINH, STSMINLH: Atomic signed minimum on halfword in memory, without return:
                   an alias of LDSMINH, LDSMINAH, LDSMINALH, LDSMINLH.

STTR: Store Register (unprivileged).

STTRB: Store Register Byte (unprivileged).

STTRH: Store Register Halfword (unprivileged).

STUMAX, STUMAXL: Atomic unsigned maximum on word or doubleword in memory, without
                 return: an alias of LDUMAX, LDUMAXA, LDUMAXAL, LDUMAXL.

STUMAXB, STUMAXLB: Atomic unsigned maximum on byte in memory, without return: an
                   alias of LDUMAXB, LDUMAXAB, LDUMAXALB, LDUMAXLB.

STUMAXH, STUMAXLH: Atomic unsigned maximum on halfword in memory, without return:
                   an alias of LDUMAXH, LDUMAXAH, LDUMAXALH, LDUMAXLH.

STUMIN, STUMINL: Atomic unsigned minimum on word or doubleword in memory, without
                 return: an alias of LDUMIN, LDUMINA, LDUMINAL, LDUMINL.

STUMINB, STUMINLB: Atomic unsigned minimum on byte in memory, without return: an
                   alias of LDUMINB, LDUMINAB, LDUMINALB, LDUMINLB.

STUMINH, STUMINLH: Atomic unsigned minimum on halfword in memory, without return:
                   an alias of LDUMINH, LDUMINAH, LDUMINALH, LDUMINLH.

STUR: Store Register (unscaled).

STURB: Store Register Byte (unscaled).

STURH: Store Register Halfword (unscaled).

STXP: Store Exclusive Pair of registers.

STXR: Store Exclusive Register.

STXRB: Store Exclusive Register Byte.

STXRH: Store Exclusive Register Halfword.

STZ2G: Store Allocation Tags, Zeroing.

STZG: Store Allocation Tag, Zeroing.

STZGM: Store Tag and Zero Multiple.

SUB (extended register): Subtract (extended register).

SUB (immediate): Subtract (immediate).

SUB (shifted register): Subtract (shifted register).

SUBG: Subtract with Tag.

SUBP: Subtract Pointer.

SUBPS: Subtract Pointer, setting Flags.

SUBPT: Subtract checked pointer.

SUBS (extended register): Subtract (extended register), setting flags.

SUBS (immediate): Subtract (immediate), setting flags.

SUBS (shifted register): Subtract (shifted register), setting flags.

SVC: Supervisor Call.

SWP, SWPA, SWPAL, SWPL: Swap word or doubleword in memory.

SWPB, SWPAB, SWPALB, SWPLB: Swap byte in memory.

SWPH, SWPAH, SWPALH, SWPLH: Swap halfword in memory.

SWPP, SWPPA, SWPPAL, SWPPL: Swap quadword in memory.

SXTB: Signed Extend Byte: an alias of SBFM.

SXTH: Sign Extend Halfword: an alias of SBFM.

SXTW: Sign Extend Word: an alias of SBFM.

SYS: System instruction.

SYSL: System instruction with result.

SYSP: 128-bit System instruction.

TBNZ: Test bit and Branch if Nonzero.

TBZ: Test bit and Branch if Zero.

TCANCEL: Cancel current transaction.

TCOMMIT: Commit current transaction.

TLBI: TLB Invalidate operation: an alias of SYS.

TLBIP: TLB Invalidate Pair operation: an alias of SYSP.

TRCIT: Trace Instrumentation: an alias of SYS.

TSB: Trace Synchronization Barrier.

TST (immediate): Test bits (immediate): an alias of ANDS (immediate).

TST (shifted register): Test (shifted register): an alias of ANDS
                        (shifted register). TSTART: Start transaction.

TTEST: Test transaction state.

UBFIZ: Unsigned Bitfield Insert in Zero: an alias of UBFM.

UBFM: Unsigned Bitfield Move.

UBFX: Unsigned Bitfield Extract: an alias of UBFM.

UDF: Permanently Undefined.

UDIV: Unsigned Divide.

UMADDL: Unsigned Multiply-Add Long.

UMAX (immediate): Unsigned Maximum (immediate).

UMAX (register): Unsigned Maximum (register).

UMIN (immediate): Unsigned Minimum (immediate).

UMIN (register): Unsigned Minimum (register).

UMNEGL: Unsigned Multiply-Negate Long: an alias of UMSUBL.

UMSUBL: Unsigned Multiply-Subtract Long.

UMULH: Unsigned Multiply High.

UMULL: Unsigned Multiply Long: an alias of UMADDL.

UXTB: Unsigned Extend Byte: an alias of UBFM.

UXTH: Unsigned Extend Halfword: an alias of UBFM.

WFE: Wait For Event.

WFET: Wait For Event with Timeout.

WFI: Wait For Interrupt.

WFIT: Wait For Interrupt with Timeout.

XAFLAG: Convert floating-point condition flags from external format to Arm format.

XPACD, XPACI, XPACLRI: Strip Pointer Authentication Code.

YIELD: YIELD.

----------------------------------------------------------------------------------------
