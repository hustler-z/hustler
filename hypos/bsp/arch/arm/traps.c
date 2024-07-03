/**
 * Hustler's Project
 *
 * File:  traps.c
 * Date:  2024/05/22
 * Usage:
 */

#include <asm/page.h>
#include <asm/hcpu.h>
#include <asm/sysregs.h>
#include <asm/barrier.h>
#include <asm/define.h>
#include <asm-generic/esr.h>
#include <common/compiler.h>
#include <common/traps.h>
#include <bsp/debug.h>
#include <bsp/panic.h>
#include <bsp/percpu.h>

// --------------------------------------------------------------
static int debug_stack_lines = 17;
static int stack_words_per_line = 2;

char *bad_tags[] = {
    "Synchronous Abort",
    "IRQ",
    "FIQ",
    "Error"
};

enum bad_code {
    SYNC_CODE = 0,
    IRQ_CODE,
    FIQ_CODE,
    ERROR_CODE,
};

/* Access Flag Fault {访问标志故障}
 *
 * If hardware management of the Access Flag is disabled for a
 * stage of translation, an access to Page or Block with the
 * Access flag bit not set in the descriptor will generate an
 * Access Flag fault.
 *
 * Permission Fault
 *
 * If hardware management of the dirty state is disabled for a
 * stage of translation, an access to a Page or Block will ignore
 * the Dirty Bit Modifier in the descriptor might generate a
 * Permission fault, depending on the values of the access
 * permission bits in the descriptor.
 */
static char *__fault_status_code(u32 fsc, int *level)
{
    char *msg = NULL;

    switch (fsc & ESR_ELx_FSC) {
        case ESR_ELx_FSC_ADDRSx ... ESR_ELx_FSC_ADDRSx + 3:
        msg = "Address Size Fault";
        *level = fsc & ESR_ELx_FSC_MASK;
        break;
    case ESR_ELx_FSC_ADDR_N1:
        msg = "Address Size Fault (Level -1)";
        break;
    case ESR_ELx_FSC_ADDR_N2:
        msg = "Address Size Fault (Level -2)";
        break;
    case ESR_ELx_FSC_FAULTx ... ESR_ELx_FSC_FAULTx + 3:
        msg = "Translation Fault";
        *level = fsc & ESR_ELx_FSC_MASK;
        break;
    case ESR_ELx_FSC_TRAN_N1:
        msg = "Translation Fault (Level -1)";
        break;
    case ESR_ELx_FSC_TRAN_N2:
        msg = "Translation Fault (Level -2)";
        break;
    case ESR_ELx_FSC_ACCESSx ... ESR_ELx_FSC_ACCESSx + 3:
        msg = "Access Flag Fault";
        *level = fsc & ESR_ELx_FSC_MASK;
        break;
    case ESR_ELx_FSC_PERMx ... ESR_ELx_FSC_PERMx + 3:
        msg = "Permission Fault";
        *level = fsc & ESR_ELx_FSC_MASK;
        break;
    case ESR_ELx_FSC_EXTABT:
        msg = "Synchronous External Abort";
        break;
    case ESR_ELx_FSC_EXTABT_N1:
        msg = "Synchronous External Abort (Level -1)";
        break;
    case ESR_ELx_FSC_EXTABT_N2:
        msg = "Synchronous External Abort (Level -2)";
        break;
    case ESR_ELx_FSC_PARITY:
        msg = "Memory Access Synchronous Parity Error";
        break;
    case ESR_ELx_FSC_SERROR:
        msg = "Memory Access Asynchronous Parity Error";
        break;
    case ESR_ELx_FSC_PARITY_N1:
        msg = "Memory Access Asynchronous Parity Error (Level -1)";
        break;
    case ESR_ELx_FSC_EXTABTx ... ESR_ELx_FSC_EXTABTx + 3:
        msg = "Sync. Ext. Abort Translation Table";
        *level = fsc & ESR_ELx_FSC_MASK;
        break;
    case ESR_ELx_FSC_PARITYx ... ESR_ELx_FSC_PARITYx + 3:
        msg = "Sync. Parity. Error Translation Table";
        *level = fsc & ESR_ELx_FSC_MASK;
        break;
    case ESR_ELx_FSC_PROT:
        msg = "Granule Protection Fault";
        break;
    case ESR_ELx_FSC_PROT_N1:
        msg = "Granule Protection Fault (Level -1)";
        break;
    case ESR_ELx_FSC_PROT_N2:
        msg = "Granule Protection Fault (Level -2)";
        break;
    case ESR_ELx_FSC_PROTx ... ESR_ELx_FSC_PROTx + 3:
        msg = "Granule Protection Fault. Translation Table";
        *level = fsc & ESR_ELx_FSC_MASK;
        break;
    case ESR_ELx_FSC_ALIGN:
        msg = "Alignment Fault";
        break;
    case ESR_ELx_FSC_LKD:
        msg = "Implementation Fault: Lockdown Abort";
        break;
    case ESR_ELx_FSC_TLB_CFLCT:
        msg = "TLB Conflict Abort";
        break;
    case ESR_ELx_FSC_NO_AHUF:
        msg = "Unsupported Atomic Hardware Update Fault";
        break;
    case ESR_ELx_FSC_NO_EAA:
        msg = "Unsupported Exclusive or Atomic Access";
        break;
    default:
        msg = "Unknown Failure";
        break;
    }

    return msg;
}

static const char *fsc_level_str(int level)
{
    switch (level) {
    case -1:
        return "";
    case 1:
        return " at Level 1 ";
    case 2:
        return " at Level 2 ";
    case 3:
        return " at Level 3 ";
    default:
        return " (Invalid Level) ";
    }
}

static void dump_far(unsigned long esr)
{
    unsigned long el, far;
    int level;
    char *eclass = NULL, *extra = NULL, *el_code = NULL;
    char wnr = 'N';
    union hcpu_esr hesr = { .bits = esr };

    switch (hesr.ec) {
    case ESR_ELx_EC_UNKNOWN:
        eclass = "Unknown Reason";
        break;
    case ESR_ELx_EC_WFx:
        eclass = "Trapped WF* Instruction Execution";
        break;
    case ESR_ELx_EC_CP15_32:
    case ESR_ELx_EC_CP15_64:
    case ESR_ELx_EC_CP14_MR:
    case ESR_ELx_EC_CP14_LS:
    case ESR_ELx_EC_FP_ASIMD:
    case ESR_ELx_EC_CP10_ID:
        eclass = "Not Implemented";
        break;
    case ESR_ELx_EC_PAC:
        eclass = "Trapped Pointer Authentication";
        break;
    case ESR_ELx_EC_CP14_64:
        eclass = "Not Implemented";
        break;
    case ESR_ELx_EC_BTI:
        eclass = "Branch Target Exception";
        break;
    case ESR_ELx_EC_ILL:
        eclass = "Illegal Excecution State";
        break;
    case ESR_ELx_EC_SVC32:
        eclass = "SVC Instruction Excecution. AArch32";
        break;
    case ESR_ELx_EC_HVC32:
        eclass = "HVC Instruction Excecution. AArch32";
        break;
    case ESR_ELx_EC_SMC32:
        eclass = "SMC Instruction Excecution. AArch32";
        break;
    case ESR_ELx_EC_SVC64:
        eclass = "SVC Instruction Excecution. AArch64";
        break;
    case ESR_ELx_EC_HVC64:
        eclass = "HVC Instruction Excecution. AArch64";
        break;
    case ESR_ELx_EC_SMC64:
        eclass = "SMC Instruction Excecution. AArch64";
        break;
    case ESR_ELx_EC_SYS64:
        eclass = "MSR/MRS. AArch64";
        break;
    case ESR_ELx_EC_SVE:
        eclass = "SVE Instruction";
        break;
    case ESR_ELx_EC_ERET:
        eclass = "Trapped ERET. ERETAA. ERETAB";
        break;
    case ESR_ELx_EC_FPAC:
        eclass = "Pointer Authentication Failure";
        break;
    case ESR_ELx_EC_IMP_DEF:
        eclass = "Implementation Defined";
        break;
    case ESR_ELx_EC_IABT_LOW:
        eclass = "Instruction Abort. Lower EL";
        break;
    case ESR_ELx_EC_IABT_CUR:
        eclass = "Instruction Abort. Current EL";
        break;
    case ESR_ELx_EC_PC_ALIGN:
        eclass = "PC Alignment Fault Exception";
        break;
    case ESR_ELx_EC_DABT_LOW:
        eclass = "Data Abort. Lower EL";
        break;
    case ESR_ELx_EC_DABT_CUR:
        eclass = "Data Abort. Current EL";
        break;
    case ESR_ELx_EC_SP_ALIGN:
        eclass = "SP Alignment Fault Exception";
        break;
    case ESR_ELx_EC_MEM_OPT:
        eclass = "Memory Operation Exception";
        break;
    case ESR_ELx_EC_FP_EXC32:
        eclass = "Trapped Floating-point Exception. AArch32";
        break;
    case ESR_ELx_EC_FP_EXC64:
        eclass = "Trapped Floating-point Exception. AArch64";
        break;
    case ESR_ELx_EC_SERROR:
        eclass = "Serror Exception";
        break;
    case ESR_ELx_EC_BREAKPT_LOW:
        eclass = "Breakpoint Exception. Lower EL";
        break;
    case ESR_ELx_EC_BREAKPT_CUR:
        eclass = "Breakpoint Exception. Current EL";
        break;
    case ESR_ELx_EC_SOFTSTP_LOW:
        eclass = "Software Step Exception. Lower EL";
        break;
    case ESR_ELx_EC_SOFTSTP_CUR:
        eclass = "Software Step Exception. Current EL";
        break;
    case ESR_ELx_EC_WATCHPT_LOW:
        eclass = "Watchpoint. Lower EL";
        break;
    case ESR_ELx_EC_WATCHPT_CUR:
        eclass = "Watchpoint. Current EL";
        break;
    case ESR_ELx_EC_BKPT32:
        eclass = "Not Implemented";
        break;
    case ESR_ELx_EC_VECTOR32:
        eclass = "Vector Catch Exception. AArch32";
        break;
    case ESR_ELx_EC_BRK64:
        eclass = "BRK Instruction Execution. AArch64";
        return;
    case ESR_ELx_EC_MAX:
        eclass = "Not Implemented";
        break;
    default:
        return;
    }

    asm("mrs %0, CurrentEL": "=r" (el));

    switch (el >> 2) {
    case 1:
        asm("mrs %0, FAR_EL1": "=r" (far));
        break;
    case 2:
        asm("mrs %0, FAR_EL2": "=r" (far));
        break;
    default:
        return;
    }

    switch (el) {
    case ARM64_EL0T:
        el_code = "EL0t";
        break;
    case ARM64_EL1T:
        el_code = "EL1t";
        break;
    case ARM64_EL1H:
        el_code = "EL1h";
        break;
    case ARM64_EL2T:
        el_code = "EL2t";
        break;
    case ARM64_EL2H:
        el_code = "EL2h";
        break;
    case ARM64_EL3T:
        el_code = "EL3t";
        break;
    case ARM64_EL3H:
        el_code = "EL3h";
        break;
    default:
        el_code = "N/A";
        break;
    }

    if (hesr.ec == ESR_ELx_EC_DABT_CUR ||
        hesr.ec == ESR_ELx_EC_DABT_LOW) {
        extra = __fault_status_code(hesr.iss, &level);
        wnr = hesr.dabt.wnr ? 'W' : 'R';
    }

    if (hesr.ec == ESR_ELx_EC_IABT_CUR ||
            hesr.ec == ESR_ELx_EC_IABT_LOW)
        extra = __fault_status_code(hesr.iss, &level);

    MSGI("@_@\n");
    MSGI("ESR     %016lx (%s <%s>  IL %u Bits  Level %u)\n"
         "ISS     %08x %s [%c]\n"
         "FAR     %016lx\n",
         hesr.bits, eclass, el_code, hesr.len ? 32 : 16, level,
         hesr.iss, extra, (wnr == 'N') ? 'N' : wnr, far);
}

#define __void__(x)     ((void *)(unsigned long)(x))

static void dump_trace(const struct hcpu_regs *regs)
{
    register_t *frame, next, addr, low, high;

    MSGI("Call Trace\n");

    MSGI("    [<%p>]  %pS (PC)\n", __void__(regs->pc),
            __void__(regs->pc));
    MSGI("    [<%p>]  %pS (LR)\n", __void__(regs->lr),
            __void__(regs->lr));

    /* Bounds for range of valid frame pointer.
     */
    low  = (register_t)((register_t *)regs->sp);
    high = (low & ~(STACK_SIZE - 1)) +
        (STACK_SIZE - sizeof(struct hcpu));

    /* The initial frame pointer. */
    next = regs->fp;

    for (;;) {
        if ((next < low) || (next >= high))
            break;

        frame = (register_t *)next;
        next  = frame[0];
        addr  = frame[1];

        MSGI("    [<%p>]  %pS\n", __void__(addr),
                __void__(addr));

        low = (register_t)&frame[1];
    }

    MSGI("@_@\n");
}

static void dump_stack(const struct hcpu_regs *regs)
{
    register_t *stack = (register_t *)regs->sp, addr;
    int i;

    MSGI("Stack Trace\n  ");

    for (i = 0; i < (debug_stack_lines * stack_words_per_line); i++) {
        if (!((long)stack & (STACK_SIZE - BYTES_PER_LONG)))
            break;
        if ((i != 0) && (!(i % stack_words_per_line)))
            MSGI("\n  ");
        addr = *stack++;
        MSGI("    %p", __void__(addr));
    }

    if (!i)
        MSGI("Stack Empty");
    MSGI("\n");

    dump_trace(regs);
}

void dump_regs(const struct hcpu_regs *regs)
{
    MSGI("@_@\n");
    MSGI("     CPSR                 %016lx\n",
            regs->cpsr);
    MSGI("  ESR_EL2                 %016lx\n",
            READ_SYSREG(ESR_EL2));
    MSGI("TTBR0_EL2                 %016lx\n",
            READ_SYSREG(TTBR0_EL2));
    MSGI("SCTLR_EL2                 %016lx\n",
            READ_SYSREG(SCTLR_EL2));
    MSGI("  HCR_EL2                 %016lx\n",
            READ_SYSREG(HCR_EL2));
    MSGI(" VTCR_EL2                 %016lx\n",
            READ_SYSREG(VTCR_EL2));
    MSGI("VTTBR_EL2                 %016lx\n",
            READ_SYSREG(VTTBR_EL2));
    MSGI("@_@\n");
    MSGI("elr %016lx   lr %016lx\n", regs->pc,  regs->lr);
    MSGI(" x0 %016lx   x1 %016lx\n", regs->x0,  regs->x1);
    MSGI(" x2 %016lx   x3 %016lx\n", regs->x2,  regs->x3);
    MSGI(" x4 %016lx   x5 %016lx\n", regs->x4,  regs->x5);
    MSGI(" x6 %016lx   x7 %016lx\n", regs->x6,  regs->x7);
    MSGI(" x8 %016lx   x9 %016lx\n", regs->x8,  regs->x9);
    MSGI("x10 %016lx  x11 %016lx\n", regs->x10, regs->x11);
    MSGI("x12 %016lx  x13 %016lx\n", regs->x12, regs->x13);
    MSGI("x14 %016lx  x15 %016lx\n", regs->x14, regs->x15);
    MSGI("x16 %016lx  x17 %016lx\n", regs->x16, regs->x17);
    MSGI("x18 %016lx  x19 %016lx\n", regs->x18, regs->x19);
    MSGI("x20 %016lx  x21 %016lx\n", regs->x20, regs->x21);
    MSGI("x22 %016lx  x23 %016lx\n", regs->x22, regs->x23);
    MSGI("x24 %016lx  x25 %016lx\n", regs->x24, regs->x25);
    MSGI("x26 %016lx  x27 %016lx\n", regs->x26, regs->x27);
    MSGI("x28 %016lx   fp %016lx\n", regs->x28, regs->fp);
    MSGI("@_@\n");
}

static void dump_execution_status(struct hcpu_regs *regs,
        char *tag)
{
    MSGE("------------------ [Hypervisor Crashed] ------------------\n");
    MSGI("        HYPOS F-ing Dead like a Head Shot at a Zombie  >_@\n");
    dump_far(regs->esr);
    dump_regs(regs);
    dump_stack(regs);
}

void panic_par(paddr_t par)
{
    const char *msg;
    int level = -1;
    int stage = par & PAR_STAGE2 ? 2 : 1;
    int snd_in_1st = !!(par & PAR_STAGE21);

    msg = __fault_status_code((par & PAR_FSC_MASK) >> PAR_FSC_SHIFT,
            &level);

    MSGI("PAR: %016lx %s Stage %d%s%s\n",
            par, msg,
            stage,
            snd_in_1st ? "During 2nd Stage Lookup" : "",
            fsc_level_str(level));
    exec_panic("<BUG>   Calm Down, @_<\n ");
}

// --------------------------------------------------------------
void do_bad_sync(struct hcpu_regs *regs)
{
    local_irq_enable();
    dump_execution_status(regs, bad_tags[SYNC_CODE]);
    trap_panic("<BUG>   Bad <%s> Got Busted, MothaF*cka      >_@",
            bad_tags[SYNC_CODE]);
}

/*
 * do_bad_irq handles the impossible case in the Irq vector.
 */
void do_bad_irq(struct hcpu_regs *regs)
{
    local_irq_enable();
    dump_execution_status(regs, bad_tags[IRQ_CODE]);
    trap_panic("<BUG>   Bad <%s> Got Busted, MothaF*cka      >_@",
            bad_tags[IRQ_CODE]);
}

/*
 * do_bad_fiq handles the impossible case in the Fiq vector.
 */
void do_bad_fiq(struct hcpu_regs *regs)
{
    local_irq_enable();
    dump_execution_status(regs, bad_tags[FIQ_CODE]);
    trap_panic("<BUG>   Bad <%s> Got Busted, MothaF*cka      >_@",
            bad_tags[FIQ_CODE]);
}

/*
 * do_bad_error handles the impossible case in the Error vector.
 */
void do_bad_error(struct hcpu_regs *regs)
{
    local_irq_enable();
    dump_execution_status(regs, bad_tags[ERROR_CODE]);
    trap_panic("<BUG>   Bad <%s> Got Busted, MothaF*cka      >_@",
            bad_tags[ERROR_CODE]);
}

/*
 * do_sync handles the Synchronous Abort exception.
 */
void do_sync(struct hcpu_regs *regs)
{
    MSGH("<%s> Fired\n", __func__);
}

/*
 * do_irq handles the Irq exception.
 */
void do_irq(struct hcpu_regs *regs)
{
    MSGH("<%s> Fired\n", __func__);
}

/*
 * do_fiq handles the Fiq exception.
 */
void do_fiq(struct hcpu_regs *regs)
{
    MSGH("<%s> Fired\n", __func__);
}

/*
 * do_error handles the Error exception.
 * Errors are more likely to be processor specific,
 * it is defined with weak attribute and can be redefined
 * in processor specific code.
 */
void __weak do_error(struct hcpu_regs *regs)
{
    MSGH("<%s> Fired\n", __func__);
}
// --------------------------------------------------------------
