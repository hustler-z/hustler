/**
 * Hustler's Project
 *
 * File:  cpu.c
 * Date:  2024/07/15
 * Usage: decode intructions and mmio implementation
 */

#include <bsp/config.h>
#include <org/vreg.h> /* XXX: Can't be modified */
#include <lib/define.h>
#include <lib/sort.h>

#if IS_IMPLEMENTED(__INSTR_IMPL)
// --------------------------------------------------------------

/* XXX: ARMv8 Instruction Encoding
 *
 * 31 30 29  27 26 25  23   21 20              11   9         4       0
 * +------------------------------------------------------------------+
 * |size|1 1 1 |V |0 0 |opc |0 |      imm9     |0 1 |  Rn     |  Rt   |
 * +------------------------------------------------------------------+
 *
 * XXX: MMIO - Memory-Mapped IO
 *
 *      A method of performing input/output (I/O) between the CPU and
 *      peripheral devices in a computer.
 *
 *      Memory-mapped I/O uses the same address space to address both
 *      memory and I/O devices. The memory and registers of the I/O
 *      devices are mapped to (associated with) address values.
 *
 *      when an address is accessed by the CPU, it may refer to a
 *      portion of physical RAM, or it can instead refer to memory
 *      of the I/O device. Thus, the CPU instructions used to access
 *      the memory can also be used for accessing devices. Each I/O
 *      device monitors the CPU's address bus and responds to any
 *      CPU access of an address assigned to that device, connecting
 *      the data bus to the desired device's hardware register.
 * --------------------------------------------------------------
 *
 *
 */
union instr {
    u32 value;
    struct {
        unsigned int rt:5;     /* Rt register */
        unsigned int rn:5;     /* Rn register */
        unsigned int fixed1:2; /* value == 01b */
        signed   int imm9:9;   /* imm9 */
        unsigned int fixed2:1; /* value == 0b */
        unsigned int opc:2;    /* opc */
        unsigned int fixed3:2; /* value == 00b */
        unsigned int v:1;      /* vector */
        unsigned int fixed4:3; /* value == 111b */
        unsigned int size:2;   /* size */
    } ldr_str;
};

static void update_dabt(struct hcpu_dabt *dabt, int reg,
                        uint8_t size, bool sign)
{
    dabt->reg = reg;
    dabt->size = size;
    dabt->sign = sign;
}

static int decode_thumb2(register_t pc, struct hcpu_dabt *dabt,
                         u16 hw1)
{
    u16 hw2;
    u16 rt;

    if (raw_copy_from_guest(&hw2, (void *)(pc + 2), sizeof (hw2)))
        return -EFAULT;

    rt = (hw2 >> 12) & 0xf;

    switch ((hw1 >> 9) & 0xf) {
    case 12:
    {
        bool sign = (hw1 & (1u << 8));
        bool load = (hw1 & (1u << 4));

        if ((hw1 & 0x0110) == 0x0100)
            goto bad_thumb2;

        if ((hw1 & 0x0070) == 0x0070)
            goto bad_thumb2;

        if (rt == 15)
            goto bad_thumb2;

        if (!load && sign)
            goto bad_thumb2;

        update_dabt(dabt, rt, (hw1 >> 5) & 3, sign);

        break;
    }
    default:
        goto bad_thumb2;
    }

    return 0;

bad_thumb2:
    MSGQ(true, "unhandled THUMB2 instruction 0x%x%x\n", hw1, hw2);

    return 1;
}

static int decode_arm64(register_t pc, mmio_info_t *info)
{
    union instr opcode = {0};
    struct hcpu_dabt *dabt = &info->dabt;
    struct instr_details *dabt_instr = &info->dabt_instr;

    if (raw_copy_from_guest(&opcode.value,
                            (void * __user)pc, sizeof (opcode))) {
        MSGQ(true, "Could not copy the instruction from PC\n");
        return 1;
    }

    if ((opcode.ldr_str.rn == opcode.ldr_str.rt) &&
        (opcode.ldr_str.rn != 31)) {
        MSGQ(true, "Rn should not be equal to Rt except for r31\n");
        goto bad_loadstore;
    }

    if ((opcode.value & POST_INDEX_FIXED_MASK)
        != POST_INDEX_FIXED_VALUE) {
        MSGQ(true, "Decoding instruction 0x%x "
                   "is not supported\n", opcode.value);
        goto bad_loadstore;
    }

    if (opcode.ldr_str.v != 0) {
        MSGQ(true, "ldr/str post indexing for vector "
                   "types are not supported\n");
        goto bad_loadstore;
    }

    if (opcode.ldr_str.opc == 0)
        dabt->write = 1;
    else if (opcode.ldr_str.opc == 1)
        dabt->write = 0;
    else {
        MSGQ(true, "Decoding ldr/str post indexing is "
                   "not supported for this variant\n");
        goto bad_loadstore;
    }

    MSGQ(true, "opcode->ldr_str.rt = 0x%x, "
               "opcode->ldr_str.size = 0x%x, "
               "opcode->ldr_str.imm9 = %d\n",
               opcode.ldr_str.rt, opcode.ldr_str.size,
               opcode.ldr_str.imm9);

    update_dabt(dabt, opcode.ldr_str.rt,
                opcode.ldr_str.size, false);

    dabt_instr->state = INSTR_LDR_STR_POSTINDEXING;
    dabt_instr->rn = opcode.ldr_str.rn;
    dabt_instr->imm9 = opcode.ldr_str.imm9;
    dabt->valid = 1;

    return 0;

 bad_loadstore:
    MSGQ(true, "unhandled Arm instruction 0x%x\n", opcode.value);
    return 1;
}

static int decode_thumb(register_t pc, struct hcpu_dabt *dabt)
{
    u16 instr;

    if (raw_copy_from_guest(&instr, (void * __user)pc, sizeof (instr)))
        return -EFAULT;

    switch (instr >> 12) {
    case 5:
    {
        /* Load/Store register */
        u16 opB = (instr >> 9) & 0x7;
        int reg = instr & 7;

        switch (opB & 0x3) {
        case 0: /* Non-signed word */
            update_dabt(dabt, reg, 2, false);
            break;
        case 1: /* Non-signed halfword */
            update_dabt(dabt, reg, 1, false);
            break;
        case 2: /* Non-signed byte */
            update_dabt(dabt, reg, 0, false);
            break;
        case 3: /* Signed byte */
            update_dabt(dabt, reg, 0, true);
            break;
        }

        break;
    }
    case 6:
        /* Load/Store word immediate offset */
        update_dabt(dabt, instr & 7, 2, false);
        break;
    case 7:
        /* Load/Store byte immediate offset */
        update_dabt(dabt, instr & 7, 0, false);
        break;
    case 8:
        /* Load/Store halfword immediate offset */
        update_dabt(dabt, instr & 7, 1, false);
        break;
    case 9:
        /* Load/Store word sp offset */
        update_dabt(dabt, (instr >> 8) & 7, 2, false);
        break;
    case 14:
        if ( instr & (1 << 11) )
            return decode_thumb2(pc, dabt, instr);
        goto bad_thumb;
    case 15:
        return decode_thumb2(pc, dabt, instr);
    default:
        goto bad_thumb;
    }

    return 0;

bad_thumb:
    MSGQ(true, "unhandled THUMB instruction 0x%x\n", instr);
    return 1;
}

int decode_instruction(const struct hcpu_regs *regs,
                       mmio_info_t *info)
{
    if (is_32bit_hypos(current->hypos) && regs->cpsr & PSR_THUMB)
        return decode_thumb(regs->pc, &info->dabt);

    if (!regs_mode_is_32bit(regs))
        return decode_arm64(regs->pc, info);

    MSGQ(true, "unhandled ARM instruction\n");

    return 1;
}

// --------------------------------------------------------------

static enum io_state handle_read(const struct mmio_handler *handler,
                                 struct vcpu *v,
                                 mmio_info_t *info)
{
    const struct hcpu_dabt dabt = info->dabt;
    struct hcpu_regs *regs = guest_hcpu_regs();

    register_t r = 0;

    if (!handler->ops->read(v, info, &r, handler->priv))
        return IO_ABORT;

    r = sign_extend(dabt, r);

    set_user_reg(regs, dabt.reg, r);

    return IO_HANDLED;
}

static enum io_state
handle_write(const struct mmio_handler *handler, struct vcpu *v,
             mmio_info_t *info)
{
    const struct hcpu_dabt dabt = info->dabt;
    struct hcpu_regs *regs = guest_hcpu_regs();
    int ret;

    ret = handler->ops->write(v, info,
                              get_user_reg(regs, dabt.reg),
                              handler->priv);
    return ret ? IO_HANDLED : IO_ABORT;
}

/* This function assumes that mmio regions are not overlapped */
static int cmp_mmio_handler(const void *key, const void *elem)
{
    const struct mmio_handler *handler0 = key;
    const struct mmio_handler *handler1 = elem;

    if (handler0->addr < handler1->addr)
        return -1;

    if (handler0->addr >= (handler1->addr + handler1->size))
        return 1;

    return 0;
}

static void swap_mmio_handler(void *_a, void *_b, size_t size)
{
    struct mmio_handler *a = _a, *b = _b;

    SWAP(*a, *b);
}

static const struct mmio_handler
*find_mmio_handler(struct hypos *d, hpa_t gpa)
{
    struct vmmio *vmmio = &h->arch.vmmio;
    struct mmio_handler key = {.addr = gpa};
    const struct mmio_handler *handler;

    read_lock(&vmmio->lock);
    handler = bsearch(&key, vmmio->handlers, vmmio->num_entries,
                      sizeof(*handler), cmp_mmio_handler);
    read_unlock(&vmmio->lock);

    return handler;
}

void try_decode_instruction(const struct hcpu_regs *regs,
                            mmio_info_t *info)
{
    int rc;

    if (info->dabt.valid) {
        info->dabt_instr.state = INSTR_VALID;

        if (check_workaround_766422() && (regs->cpsr & PSR_THUMB) &&
            info->dabt.write) {
            rc = decode_instruction(regs, info);
            if (rc) {
                MSGQ(true, "Unable to decode instruction\n");
                info->dabt_instr.state = INSTR_ERROR;
            }
        }
        return;
    }

    if (info->dabt.s1ptw) {
        info->dabt_instr.state = INSTR_ERROR;
        return;
    }

    if (info->dabt.cache) {
        info->dabt_instr.state = INSTR_CACHE;
        return;
    }

    rc = decode_instruction(regs, info);
    if (rc) {
        MSGQ(true, "Unable to decode instruction\n");
        info->dabt_instr.state = INSTR_ERROR;
    }
}

enum io_state try_handle_mmio(struct hcpu_regs *regs,
                              mmio_info_t *info)
{
    struct vcpu *v = current;
    const struct mmio_handler *handler = NULL;
    int rc;

    ASSERT(info->dabt.ec == HSR_EC_DATA_ABORT_LOWER_EL);

    if (!(info->dabt.valid ||
        (info->dabt_instr.state == INSTR_CACHE))) {
        ASSERT_UNREACHABLE();
        return IO_ABORT;
    }

    handler = find_mmio_handler(v->hypos, info->gpa);
    if (!handler) {
        rc = try_fwd_ioserv(regs, v, info);
        if (rc == IO_HANDLED)
            return handle_ioserv(regs, v);

        return rc;
    }

    if (info->dabt_instr.state == INSTR_CACHE)
        return IO_HANDLED;

    if (info->dabt.write)
        return handle_write(handler, v, info);
    else
        return handle_read(handler, v, info);
}

void register_mmio_handler(struct hypos *h,
                           const struct mmio_handler_ops *ops,
                           hpa_t addr, hpa_t size, void *priv)
{
    struct vmmio *vmmio = &h->arch.vmmio;
    struct mmio_handler *handler;

    BUG_ON(vmmio->num_entries >= vmmio->max_num_entries);

    write_lock(&vmmio->lock);

    handler = &vmmio->handlers[vmmio->num_entries];

    handler->ops = ops;
    handler->addr = addr;
    handler->size = size;
    handler->priv = priv;

    vmmio->num_entries++;

    /* Sort mmio handlers in ascending order
     * based on base address */
    sort(vmmio->handlers, vmmio->num_entries,
         sizeof(struct mmio_handler),
         cmp_mmio_handler, swap_mmio_handler);

    write_unlock(&vmmio->lock);
}

int hypos_io_init(struct hypos *h, unsigned int max_count)
{
    rwlock_init(&h->arch.vmmio.lock);
    h->arch.vmmio.num_entries = 0;
    h->arch.vmmio.max_num_entries = max_count;
    h->arch.vmmio.handlers =
        zalloc_array(struct mmio_handler, max_count);
    if (!h->arch.vmmio.handlers)
        return -ENOMEM;

    return 0;
}

void hypos_io_free(struct hypos *h)
{
    free(h->arch.vmmio.handlers);
}

// --------------------------------------------------------------
#else

void register_mmio_handler(struct hypos *h,
                           const struct mmio_handler_ops *ops,
                           hpa_t addr, hpa_t size, void *priv)
{
    /* TODO: Dummy Implementation */
}

// --------------------------------------------------------------
#endif
