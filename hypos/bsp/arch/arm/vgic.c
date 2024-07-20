/**
 * Hustler's Project
 *
 * File:  vgic.c
 * Date:  2024/05/22
 * Usage:
 */

#include <org/vcpu.h>
#include <bsp/config.h>
#include <asm/barrier.h>

#if IS_IMPLEMENTED(__VGIC_IMPL)
// --------------------------------------------------------------

struct virt_its {
    struct hypos *d;
    struct list_head vits_list;
    hpa_t doorbell_address;
    unsigned int devid_bits;
    unsigned int evid_bits;
    spinlock_t vcmd_lock;
    u64 cwriter;
    u64 creadr;

    spinlock_t its_lock;
    u64 cbaser;
    u64 baser_dev, baser_coll;
    unsigned int max_collections;
    unsigned int max_devices;

    bool enabled;
};

struct vits_itte
{
    u32 vlpi;
    u16 collection;
    u16 pad;
};

typedef u16 coll_table_entry_t;
#define UNMAPPED_COLLECTION      ((coll_table_entry_t)~0)

typedef u64 dev_table_entry_t;
#define DEV_TABLE_ITT_ADDR(x) ((x) & GENMASK(51, 8))
#define DEV_TABLE_ITT_SIZE(x) (BIT(((x) & GENMASK(4, 0)) + 1, UL))
#define DEV_TABLE_ENTRY(addr, bits)                     \
        (((addr) & GENMASK(51, 8)) | (((bits) - 1) & GENMASK(4, 0)))

#define GITS_BASER_RO_MASK \
    (GITS_BASER_TYPE_MASK | (0x1fL << GITS_BASER_ENTRY_SIZE_SHIFT))

static hpa_t get_baser_phys_addr(u64 reg)
{
    if (reg & BIT(9, UL))
        return (reg & GENMASK(47, 16)) |
                ((reg & GENMASK(15, 12)) << 36);
    else
        return reg & GENMASK(47, 12);
}

static int its_set_collection(struct virt_its *its, u16 collid,
                              coll_table_entry_t vcpu_id)
{
    hpa_t addr = get_baser_phys_addr(its->baser_coll);

    BUILD_BUG_ON(BIT(sizeof(coll_table_entry_t) * 8, UL) < MAX_VIRT_CPUS);

    ASSERT(spin_is_locked(&its->its_lock));

    if (collid >= its->max_collections)
        return -ENOENT;

    return access_guest_memory_by_gpa(its->d,
                                      addr + collid * sizeof(coll_table_entry_t),
                                      &vcpu_id, sizeof(vcpu_id), true);
}

static struct vcpu *get_vcpu_from_collection(struct virt_its *its,
                                             u16 collid)
{
    hpa_t addr = get_baser_phys_addr(its->baser_coll);
    coll_table_entry_t vcpu_id;
    int ret;

    ASSERT(spin_is_locked(&its->its_lock));

    if (collid >= its->max_collections)
        return NULL;

    ret = access_guest_memory_by_gpa(its->d,
                                     addr + collid * sizeof(coll_table_entry_t),
                                     &vcpu_id, sizeof(coll_table_entry_t), false);
    if (ret)
        return NULL;

    if (vcpu_id == UNMAPPED_COLLECTION || vcpu_id >= its->d->max_vcpus)
        return NULL;

    return its->d->vcpu[vcpu_id];
}

static int its_set_itt_address(struct virt_its *its, u32 devid,
                               hpa_t itt_address, u32 nr_bits)
{
    hpa_t addr = get_baser_phys_addr(its->baser_dev);
    dev_table_entry_t itt_entry = DEV_TABLE_ENTRY(itt_address, nr_bits);

    if (devid >= its->max_devices)
        return -ENOENT;

    return access_guest_memory_by_gpa(its->d,
                                      addr + devid * sizeof(dev_table_entry_t),
                                      &itt_entry, sizeof(itt_entry), true);
}

static int its_get_itt(struct virt_its *its, u32 devid,
                       dev_table_entry_t *itt)
{
    hpa_t addr = get_baser_phys_addr(its->baser_dev);

    if (devid >= its->max_devices)
        return -EINVAL;

    return access_guest_memory_by_gpa(its->d,
                                      addr + devid * sizeof(dev_table_entry_t),
                                      itt, sizeof(*itt), false);
}

static hpa_t its_get_itte_address(struct virt_its *its,
                                    u32 devid, u32 evid)
{
    dev_table_entry_t itt;
    int ret;

    ret = its_get_itt(its, devid, &itt);
    if (ret)
        return INVALID_PADDR;

    if (evid >= DEV_TABLE_ITT_SIZE(itt) ||
        DEV_TABLE_ITT_ADDR(itt) == INVALID_PADDR)
        return INVALID_PADDR;

    return DEV_TABLE_ITT_ADDR(itt) + evid * sizeof(struct vits_itte);
}

static bool read_itte(struct virt_its *its, u32 devid, u32 evid,
                      struct vcpu **vcpu_ptr, u32 *vlpi_ptr)
{
    hpa_t addr;
    struct vits_itte itte;
    struct vcpu *vcpu;

    ASSERT(spin_is_locked(&its->its_lock));

    addr = its_get_itte_address(its, devid, evid);
    if (addr == INVALID_PADDR)
        return false;

    if (access_guest_memory_by_gpa(its->d, addr,
                                   &itte, sizeof(itte), false))
        return false;

    vcpu = get_vcpu_from_collection(its, itte.collection);
    if (!vcpu)
        return false;

    *vcpu_ptr = vcpu;
    *vlpi_ptr = itte.vlpi;
    return true;
}

static bool write_itte(struct virt_its *its, u32 devid,
                       u32 evid, u32 collid, u32 vlpi)
{
    hpa_t addr;
    struct vits_itte itte;

    ASSERT(spin_is_locked(&its->its_lock));

    addr = its_get_itte_address(its, devid, evid);
    if (addr == INVALID_PADDR)
        return false;

    itte.collection = collid;
    itte.vlpi = vlpi;

    if (access_guest_memory_by_gpa(its->d, addr, &itte, sizeof(itte), true))
        return false;

    return true;
}

static u64 its_cmd_mask_field(u64 *its_cmd, unsigned int word,
                                   unsigned int shift, unsigned int size)
{
    return (its_cmd[word] >> shift) & GENMASK(size - 1, 0);
}

#define its_cmd_get_command(cmd)        its_cmd_mask_field(cmd, 0,  0,  8)
#define its_cmd_get_deviceid(cmd)       its_cmd_mask_field(cmd, 0, 32, 32)
#define its_cmd_get_size(cmd)           its_cmd_mask_field(cmd, 1,  0,  5)
#define its_cmd_get_id(cmd)             its_cmd_mask_field(cmd, 1,  0, 32)
#define its_cmd_get_physical_id(cmd)    its_cmd_mask_field(cmd, 1, 32, 32)
#define its_cmd_get_collection(cmd)     its_cmd_mask_field(cmd, 2,  0, 16)
#define its_cmd_get_target_addr(cmd)    its_cmd_mask_field(cmd, 2, 16, 32)
#define its_cmd_get_validbit(cmd)       its_cmd_mask_field(cmd, 2, 63,  1)
#define its_cmd_get_ittaddr(cmd)        (its_cmd_mask_field(cmd, 2, 8, 44) << 8)

static int its_handle_int(struct virt_its *its, u64 *cmdptr)
{
    u32 devid = its_cmd_get_deviceid(cmdptr);
    u32 eventid = its_cmd_get_id(cmdptr);
    struct vcpu *vcpu;
    u32 vlpi;
    bool ret;

    spin_lock(&its->its_lock);
    ret = read_itte(its, devid, eventid, &vcpu, &vlpi);
    spin_unlock(&its->its_lock);
    if (!ret)
        return -1;

    if (vlpi == INVALID_LPI)
        return -1;

    vgic_vcpu_inject_lpi(its->d, vlpi);

    return 0;
}

static int its_handle_mapc(struct virt_its *its, u64 *cmdptr)
{
    u32 collid = its_cmd_get_collection(cmdptr);
    u64 rdbase = its_cmd_mask_field(cmdptr, 2, 16, 44);

    if (collid >= its->max_collections)
        return -1;

    if (rdbase >= its->d->max_vcpus)
        return -1;

    spin_lock(&its->its_lock);

    if (its_cmd_get_validbit(cmdptr))
        its_set_collection(its, collid, rdbase);
    else
        its_set_collection(its, collid, UNMAPPED_COLLECTION);

    spin_unlock(&its->its_lock);

    return 0;
}

static int its_handle_clear(struct virt_its *its, u64 *cmdptr)
{
    u32 devid = its_cmd_get_deviceid(cmdptr);
    u32 eventid = its_cmd_get_id(cmdptr);
    struct pending_irq *p;
    struct vcpu *vcpu;
    u32 vlpi;
    unsigned long flags;
    int ret = -1;

    spin_lock(&its->its_lock);

    if (!read_itte(its, devid, eventid, &vcpu, &vlpi))
        goto out_unlock;

    p = gicv3_its_get_event_pending_irq(its->d, its->doorbell_address,
                                        devid, eventid);
    if (unlikely(!p))
        goto out_unlock;

    spin_lock_irqsave(&vcpu->arch.vgic.lock, flags);

    if (!test_bit(GIC_IRQ_GUEST_VISIBLE, &p->status))
        vgic_remove_irq_from_queues(vcpu, p);

    spin_unlock_irqrestore(&vcpu->arch.vgic.lock, flags);
    ret = 0;

out_unlock:
    spin_unlock(&its->its_lock);

    return ret;
}

static int update_lpi_property(struct hypos *d, struct pending_irq *p)
{
    hpa_t addr;
    uint8_t property;
    int ret;

    if (!d->arch.vgic.rdists_enabled)
        return 0;

    addr = d->arch.vgic.rdist_propbase & GENMASK(51, 12);

    ret = access_guest_memory_by_gpa(d, addr + p->irq - LPI_OFFSET,
                                     &property, sizeof(property), false);
    if (ret)
        return ret;

    write_atomic(&p->lpi_priority, property & LPI_PROP_PRIO_MASK);

    if (property & LPI_PROP_ENABLED)
        set_bit(GIC_IRQ_GUEST_ENABLED, &p->status);
    else
        clear_bit(GIC_IRQ_GUEST_ENABLED, &p->status);

    return 0;
}

static void update_lpi_vgic_status(struct vcpu *v, struct pending_irq *p)
{
    ASSERT(spin_is_locked(&v->arch.vgic.lock));

    if (test_bit(GIC_IRQ_GUEST_ENABLED, &p->status)) {
        if (!list_empty(&p->inflight) &&
            !test_bit(GIC_IRQ_GUEST_VISIBLE, &p->status))
            gic_raise_guest_irq(v, p->irq, p->lpi_priority);
    }
    else
        gic_remove_from_lr_pending(v, p);
}

static int its_handle_inv(struct virt_its *its, u64 *cmdptr)
{
    struct hypos *d = its->d;
    u32 devid = its_cmd_get_deviceid(cmdptr);
    u32 eventid = its_cmd_get_id(cmdptr);
    struct pending_irq *p;
    unsigned long flags;
    struct vcpu *vcpu;
    u32 vlpi;
    int ret = -1;

    if (!d->arch.vgic.rdists_enabled)
        return 0;

    spin_lock(&its->its_lock);

    if (!read_itte(its, devid, eventid, &vcpu, &vlpi))
        goto out_unlock_its;

    if (vlpi == INVALID_LPI)
        goto out_unlock_its;

    p = gicv3_its_get_event_pending_irq(d, its->doorbell_address,
                                        devid, eventid);
    if (unlikely(!p))
        goto out_unlock_its;

    spin_lock_irqsave(&vcpu->arch.vgic.lock, flags);

    if (update_lpi_property(d, p))
        goto out_unlock;

    update_lpi_vgic_status(vcpu, p);

    ret = 0;

out_unlock:
    spin_unlock_irqrestore(&vcpu->arch.vgic.lock, flags);

out_unlock_its:
    spin_unlock(&its->its_lock);

    return ret;
}

static int its_handle_invall(struct virt_its *its, u64 *cmdptr)
{
    u32 collid = its_cmd_get_collection(cmdptr);
    struct vcpu *vcpu;
    struct pending_irq *pirqs[16];
    u64 vlpi = 0;          /* 64-bit to catch overflows */
    unsigned int nr_lpis, i;
    unsigned long flags;
    int ret = 0;

    ASSERT(is_hardware_hypos(its->d));

    if (!its->d->arch.vgic.rdists_enabled)
        return 0;

    spin_lock(&its->its_lock);
    vcpu = get_vcpu_from_collection(its, collid);
    spin_unlock(&its->its_lock);

    spin_lock_irqsave(&vcpu->arch.vgic.lock, flags);
    read_lock(&its->d->arch.vgic.pend_lpi_tree_lock);

    do {
        int err;

        nr_lpis = radix_tree_gang_lookup(&its->d->arch.vgic.pend_lpi_tree,
                                         (void **)pirqs, vlpi,
                                         ARRAY_SIZE(pirqs));

        for (i = 0; i < nr_lpis; i++) {
            if (pirqs[i]->lpi_vcpu_id != vcpu->vcpu_id)
                continue;

            vlpi = pirqs[i]->irq;
            err = update_lpi_property(its->d, pirqs[i]);
            if (!err)
                update_lpi_vgic_status(vcpu, pirqs[i]);
            else
                ret = err;
        }
    } while ((++vlpi < its->d->arch.vgic.nr_lpis) &&
             (nr_lpis == ARRAY_SIZE(pirqs)));

    read_unlock(&its->d->arch.vgic.pend_lpi_tree_lock);
    spin_unlock_irqrestore(&vcpu->arch.vgic.lock, flags);

    return ret;
}

static int its_discard_event(struct virt_its *its,
                             u32 vdevid, u32 vevid)
{
    struct pending_irq *p;
    unsigned long flags;
    struct vcpu *vcpu;
    u32 vlpi;

    ASSERT(spin_is_locked(&its->its_lock));

    if (!read_itte(its, vdevid, vevid, &vcpu, &vlpi))
        return -ENOENT;

    if (vlpi == INVALID_LPI)
        return -ENOENT;

    spin_lock_irqsave(&vcpu->arch.vgic.lock, flags);

    write_lock(&its->d->arch.vgic.pend_lpi_tree_lock);
    p = radix_tree_delete(&its->d->arch.vgic.pend_lpi_tree, vlpi);
    write_unlock(&its->d->arch.vgic.pend_lpi_tree_lock);

    if (!p) {
        spin_unlock_irqrestore(&vcpu->arch.vgic.lock, flags);

        return -ENOENT;
    }

    vgic_remove_irq_from_queues(vcpu, p);
    vgic_init_pending_irq(p, INVALID_LPI);

    spin_unlock_irqrestore(&vcpu->arch.vgic.lock, flags);

    return gicv3_remove_guest_event(its->d, its->doorbell_address,
                                    vdevid, vevid);
}

static void its_unmap_device(struct virt_its *its, u32 devid)
{
    dev_table_entry_t itt;
    u64 evid;

    spin_lock(&its->its_lock);

    if (its_get_itt(its, devid, &itt))
        goto out;

    ASSERT(is_hardware_hypos(its->d));

    for (evid = 0; evid < DEV_TABLE_ITT_SIZE(itt); evid++)
        its_discard_event(its, devid, evid);

out:
    spin_unlock(&its->its_lock);
}

static int its_handle_mapd(struct virt_its *its, u64 *cmdptr)
{
    u32 devid = its_cmd_get_deviceid(cmdptr);
    unsigned int size = its_cmd_get_size(cmdptr) + 1;
    bool valid = its_cmd_get_validbit(cmdptr);
    hpa_t itt_addr = its_cmd_get_ittaddr(cmdptr);
    int ret;

    if (valid && (size > its->evid_bits))
        return -1;

    if (!valid)
        its_unmap_device(its, devid);

    if (is_hardware_hypos(its->d)) {
        ret = gicv3_its_map_guest_device(its->d,
                                         its->doorbell_address,
                                         devid,
                                         its->doorbell_address,
                                         devid,
                                         BIT(size, UL), valid);
        if (ret && valid)
            return ret;
    }

    spin_lock(&its->its_lock);

    if (valid)
        ret = its_set_itt_address(its, devid, itt_addr, size);
    else
        ret = its_set_itt_address(its, devid, INVALID_PADDR, 1);

    spin_unlock(&its->its_lock);

    return ret;
}

static int its_handle_mapti(struct virt_its *its, u64 *cmdptr)
{
    u32 devid   = its_cmd_get_deviceid(cmdptr);
    u32 eventid = its_cmd_get_id(cmdptr);
    u32 intid   = its_cmd_get_physical_id(cmdptr), _intid;
    u16 collid  = its_cmd_get_collection(cmdptr);
    struct pending_irq *pirq;
    struct vcpu *vcpu = NULL;
    int ret = -1;

    if (its_cmd_get_command(cmdptr) == GITS_CMD_MAPI)
        intid = eventid;

    spin_lock(&its->its_lock);

    if (read_itte(its, devid, eventid, &vcpu, &_intid) &&
        _intid != INVALID_LPI ) {
        spin_unlock(&its->its_lock);
        return -1;
    }

    vcpu = get_vcpu_from_collection(its, collid);
    if (!vcpu || intid >= its->d->arch.vgic.nr_lpis) {
        spin_unlock(&its->its_lock);
        return -1;
    }

    if (!write_itte(its, devid, eventid, collid, intid)) {
        spin_unlock(&its->its_lock);
        return -1;
    }

    spin_unlock(&its->its_lock);

    pirq = gicv3_assign_guest_event(its->d, its->doorbell_address,
                                    devid, eventid, intid);
    if (!pirq)
        goto out_remove_mapping;

    vgic_init_pending_irq(pirq, intid);

    ret = update_lpi_property(its->d, pirq);
    if (ret)
        goto out_remove_host_entry;

    pirq->lpi_vcpu_id = vcpu->vcpu_id;

    set_bit(GIC_IRQ_GUEST_PRISTINE_LPI, &pirq->status);

    write_lock(&its->d->arch.vgic.pend_lpi_tree_lock);
    ret = radix_tree_insert(&its->d->arch.vgic.pend_lpi_tree, intid, pirq);
    write_unlock(&its->d->arch.vgic.pend_lpi_tree_lock);

    if (!ret)
        return 0;

out_remove_host_entry:
    gicv3_remove_guest_event(its->d, its->doorbell_address, devid, eventid);

out_remove_mapping:
    spin_lock(&its->its_lock);
    write_itte(its, devid, eventid, UNMAPPED_COLLECTION, INVALID_LPI);
    spin_unlock(&its->its_lock);

    return ret;
}

static int its_handle_movi(struct virt_its *its, u64 *cmdptr)
{
    u32 devid = its_cmd_get_deviceid(cmdptr);
    u32 eventid = its_cmd_get_id(cmdptr);
    u16 collid = its_cmd_get_collection(cmdptr);
    unsigned long flags;
    struct pending_irq *p;
    struct vcpu *ovcpu, *nvcpu;
    u32 vlpi;
    int ret = -1;

    spin_lock(&its->its_lock);
    if (!read_itte(its, devid, eventid, &ovcpu, &vlpi))
        goto out_unlock;

    if (vlpi == INVALID_LPI)
        goto out_unlock;

    nvcpu = get_vcpu_from_collection(its, collid);
    if (!nvcpu)
        goto out_unlock;

    p = gicv3_its_get_event_pending_irq(its->d, its->doorbell_address,
                                        devid, eventid);
    if (unlikely(!p))
        goto out_unlock;

    spin_lock_irqsave(&ovcpu->arch.vgic.lock, flags);

    p->lpi_vcpu_id = nvcpu->vcpu_id;

    spin_unlock_irqrestore(&ovcpu->arch.vgic.lock, flags);

    if (!write_itte(its, devid, eventid, collid, vlpi))
        goto out_unlock;

    ret = 0;

out_unlock:
    spin_unlock(&its->its_lock);

    return ret;
}

static int its_handle_discard(struct virt_its *its, u64 *cmdptr)
{
    u32 devid = its_cmd_get_deviceid(cmdptr);
    u32 eventid = its_cmd_get_id(cmdptr);
    int ret;

    spin_lock(&its->its_lock);

    ret = its_discard_event(its, devid, eventid);
    if (ret)
        goto out_unlock;

    if (!write_itte(its, devid, eventid,
                    UNMAPPED_COLLECTION, INVALID_LPI))
        ret = -1;

out_unlock:
    spin_unlock(&its->its_lock);

    return ret;
}

#define ITS_CMD_BUFFER_SIZE(baser)  ((((baser) & 0xFF) + 1) << 12)
#define ITS_CMD_OFFSET(reg)         ((reg) & GENMASK(19, 5))

static void dump_its_command(u64 *command)
{
    MSGH("cmd 0x%02lx: %016lx %016lx %016lx %016lx\n",
         its_cmd_get_command(command),
         command[0], command[1], command[2], command[3]);
}

static int vgic_its_handle_cmds(struct hypos *d,
                                struct virt_its *its)
{
    hpa_t addr = its->cbaser & GENMASK(51, 12);
    u64 command[4];

    ASSERT(spin_is_locked(&its->vcmd_lock));

    if (its->cwriter >= ITS_CMD_BUFFER_SIZE(its->cbaser))
        return -1;

    while (its->creadr != its->cwriter) {
        int ret;

        ret = access_guest_memory_by_gpa(d, addr + its->creadr,
                                         command, sizeof(command),
                                         false);
        if (ret)
            return ret;

        switch (its_cmd_get_command(command)) {
        case GITS_CMD_CLEAR:
            ret = its_handle_clear(its, command);
            break;
        case GITS_CMD_DISCARD:
            ret = its_handle_discard(its, command);
            break;
        case GITS_CMD_INT:
            ret = its_handle_int(its, command);
            break;
        case GITS_CMD_INV:
            ret = its_handle_inv(its, command);
            break;
        case GITS_CMD_INVALL:
            ret = its_handle_invall(its, command);
            break;
        case GITS_CMD_MAPC:
            ret = its_handle_mapc(its, command);
            break;
        case GITS_CMD_MAPD:
            ret = its_handle_mapd(its, command);
            break;
        case GITS_CMD_MAPI:
        case GITS_CMD_MAPTI:
            ret = its_handle_mapti(its, command);
            break;
        case GITS_CMD_MOVALL:
            MSGH("vGITS: ignoring MOVALL command\n");
            break;
        case GITS_CMD_MOVI:
            ret = its_handle_movi(its, command);
            break;
        case GITS_CMD_SYNC:
            break;
        default:
            MSGH("vGITS: unhandled ITS command\n");
            dump_its_command(command);
            break;
        }

        write_u64_atomic(&its->creadr, (its->creadr + ITS_CMD_SIZE) %
                         ITS_CMD_BUFFER_SIZE(its->cbaser));

        if (ret) {
            MSGH("vGITS: ITS command error %d while handling command\n",
                 ret);
            dump_its_command(command);
        }
    }

    return 0;
}

#define GITS_IIDR_VALUE                 0x5800034C

static int vgic_v3_its_mmio_read(struct vcpu *v,
                                 mmio_info_t *info,
                                 register_t *r, void *priv)
{
    struct virt_its *its = priv;
    u64 reg;

    switch (info->gpa & 0xFFFF) {
    case VREG32(GITS_CTLR):
    {
        bool have_cmd_lock;

        if (info->dabt.size != DABT_WORD)
            goto bad_width;

        have_cmd_lock = spin_trylock(&its->vcmd_lock);
        reg = its->enabled ? GITS_CTLR_ENABLE : 0;

        if (have_cmd_lock && its->cwriter == its->creadr)
            reg |= GITS_CTLR_QUIESCENT;

        if (have_cmd_lock)
            spin_unlock(&its->vcmd_lock);

        *r = vreg_reg32_extract(reg, info);
        break;
    }
    case VREG32(GITS_IIDR):
        if (info->dabt.size != DABT_WORD)
            goto bad_width;
        *r = vreg_reg32_extract(GITS_IIDR_VALUE, info);
        break;

    case VREG64(GITS_TYPER):
        if (!vgic_reg64_check_access(info->dabt))
            goto bad_width;

        reg = GITS_TYPER_PHYSICAL;
        reg |= (sizeof(struct vits_itte) - 1) << GITS_TYPER_ITT_SIZE_SHIFT;
        reg |= (its->evid_bits - 1) << GITS_TYPER_IDBITS_SHIFT;
        reg |= (its->devid_bits - 1) << GITS_TYPER_DEVIDS_SHIFT;
        *r = vreg_reg64_extract(reg, info);
        break;
    case VRANGE32(0x0018, 0x001C):
        goto read_reserved;
    case VRANGE32(0x0020, 0x003C):
        goto read_impl_defined;
    case VRANGE32(0x0040, 0x007C):
        goto read_reserved;
    case VREG64(GITS_CBASER):
        if (!vgic_reg64_check_access(info->dabt))
            goto bad_width;
        spin_lock(&its->its_lock);
        *r = vreg_reg64_extract(its->cbaser, info);
        spin_unlock(&its->its_lock);
        break;
    case VREG64(GITS_CWRITER):
        if (!vgic_reg64_check_access(info->dabt))
            goto bad_width;

        reg = its->cwriter;
        *r = vreg_reg64_extract(reg, info);
        break;
    case VREG64(GITS_CREADR):
        if (!vgic_reg64_check_access(info->dabt))
            goto bad_width;

        reg = read_u64_atomic(&its->creadr);
        *r = vreg_reg64_extract(reg, info);
        break;
    case VRANGE64(0x0098, 0x00F8):
        goto read_reserved;
    case VREG64(GITS_BASER0):
        if (!vgic_reg64_check_access(info->dabt))
            goto bad_width;
        spin_lock(&its->its_lock);
        *r = vreg_reg64_extract(its->baser_dev, info);
        spin_unlock(&its->its_lock);
        break;
    case VREG64(GITS_BASER1):
        if (!vgic_reg64_check_access(info->dabt))
            goto bad_width;
        spin_lock(&its->its_lock);
        *r = vreg_reg64_extract(its->baser_coll, info);
        spin_unlock(&its->its_lock);
        break;
    case VRANGE64(GITS_BASER2, GITS_BASER7):
        goto read_as_zero_64;
    case VRANGE32(0x0140, 0xBFFC):
        goto read_reserved;
    case VRANGE32(0xC000, 0xFFCC):
        goto read_impl_defined;
    case VRANGE32(0xFFD0, 0xFFE4):
        goto read_impl_defined;
    case VREG32(GITS_PIDR2):
        if (info->dabt.size != DABT_WORD)
            goto bad_width;
        *r = vreg_reg32_extract(GIC_PIDR2_ARCH_GICv3, info);
        break;
    case VRANGE32(0xFFEC, 0xFFFC):
        goto read_impl_defined;
    default:
        MSGH("%pv: vGITS: unhandled read r%d offset %#04lx\n",
             v, info->dabt.reg, (unsigned long)info->gpa & 0xFFFF);
        return 0;
    }

    return 1;

read_as_zero_64:
    if (!vgic_reg64_check_access(info->dabt))
        goto bad_width;
    *r = 0;

    return 1;

read_impl_defined:
    MSGH("%pv: vGITS: RAZ on implementation defined register offset %#04lx\n",
         v, info->gpa & 0xFFFF);
    *r = 0;
    return 1;

read_reserved:
    MSGH("%pv: vGITS: RAZ on reserved register offset %#04lx\n",
         v, info->gpa & 0xFFFF);
    *r = 0;
    return 1;

bad_width:
    MSGH("vGITS: bad read width %d r%d offset %#04lx\n",
         info->dabt.size, info->dabt.reg,
         (unsigned long)info->gpa & 0xFFFF);

    return 0;
}

static unsigned int its_baser_table_size(u64 baser)
{
    unsigned int ret, page_size[4] = {KB(4), KB(16), KB(64), KB(64)};

    ret = page_size[(baser >> GITS_BASER_PAGE_SIZE_SHIFT) & 3];

    return ret * ((baser & GITS_BASER_SIZE_MASK) + 1);
}

static unsigned int its_baser_nr_entries(u64 baser)
{
    unsigned int entry_size = GITS_BASER_ENTRY_SIZE(baser);

    return its_baser_table_size(baser) / entry_size;
}

static bool vgic_v3_verify_its_status(struct virt_its *its, bool status)
{
    ASSERT(spin_is_locked(&its->its_lock));

    if (!status)
        return false;

    if (!(its->cbaser & GITS_VALID_BIT) ||
        !(its->baser_dev & GITS_VALID_BIT) ||
        !(its->baser_coll & GITS_VALID_BIT)) {
        MSGH("d%d tried to enable ITS without having the tables configured.\n",
               its->d->hypos_id);
        return false;
    }

    ASSERT(is_hardware_hypos(its->d));

    return true;
}

static void sanitize_its_base_reg(u64 *reg)
{
    u64 r = *reg;

    switch ((r >> GITS_BASER_SHAREABILITY_SHIFT) & 0x03) {
    case GIC_BASER_OuterShareable:
        r &= ~GITS_BASER_SHAREABILITY_MASK;
        r |= GIC_BASER_InnerShareable << GITS_BASER_SHAREABILITY_SHIFT;
        break;
    default:
        break;
    }

    switch ((r >> GITS_BASER_INNER_CACHEABILITY_SHIFT) & 0x07) {
    case GIC_BASER_CACHE_nCnB:
    case GIC_BASER_CACHE_nC:
        r &= ~GITS_BASER_INNER_CACHEABILITY_MASK;
        r |= GIC_BASER_CACHE_RaWb << GITS_BASER_INNER_CACHEABILITY_SHIFT;
        break;
    default:
        break;
    }

    switch ((r >> GITS_BASER_OUTER_CACHEABILITY_SHIFT) & 0x07) {
    case GIC_BASER_CACHE_SameAsInner:
    case GIC_BASER_CACHE_nC:
        break;
    default:
        r &= ~GITS_BASER_OUTER_CACHEABILITY_MASK;
        r |= GIC_BASER_CACHE_nC << GITS_BASER_OUTER_CACHEABILITY_SHIFT;
        break;
    }

    *reg = r;
}

static int vgic_v3_its_mmio_write(struct vcpu *v,
                                  mmio_info_t *info,
                                  register_t r,
                                  void *priv)
{
    struct hypos *d = v->hypos;
    struct virt_its *its = priv;
    u64 reg;
    u32 reg32;

    switch (info->gpa & 0xFFFF) {
    case VREG32(GITS_CTLR):
    {
        u32 ctlr;

        if (info->dabt.size != DABT_WORD)
            goto bad_width;

        spin_lock(&its->vcmd_lock);
        spin_lock(&its->its_lock);
        ctlr = its->enabled ? GITS_CTLR_ENABLE : 0;
        reg32 = ctlr;
        vreg_reg32_update(&reg32, r, info);

        if (ctlr ^ reg32)
            its->enabled =
                vgic_v3_verify_its_status(its, reg32 & GITS_CTLR_ENABLE);
        spin_unlock(&its->its_lock);
        spin_unlock(&its->vcmd_lock);
        return 1;
    }
    case VREG32(GITS_IIDR):
        goto write_ignore_32;
    case VREG32(GITS_TYPER):
        goto write_ignore_32;
    case VRANGE32(0x0018, 0x001C):
        goto write_reserved;
    case VRANGE32(0x0020, 0x003C):
        goto write_impl_defined;
    case VRANGE32(0x0040, 0x007C):
        goto write_reserved;
    case VREG64(GITS_CBASER):
        if (!vgic_reg64_check_access(info->dabt))
            goto bad_width;

        spin_lock(&its->its_lock);
        if (its->enabled) {
            spin_unlock(&its->its_lock);
            MSGH("vGITS: tried to change CBASER with the ITS enabled.\n");
            return 1;
        }

        reg = its->cbaser;
        vreg_reg64_update(&reg, r, info);
        sanitize_its_base_reg(&reg);

        its->cbaser = reg;
        its->creadr = 0;
        spin_unlock(&its->its_lock);

        return 1;
    case VREG64(GITS_CWRITER):
        if (!vgic_reg64_check_access(info->dabt))
            goto bad_width;

        spin_lock(&its->vcmd_lock);
        reg = ITS_CMD_OFFSET(its->cwriter);
        vreg_reg64_update(&reg, r, info);
        its->cwriter = ITS_CMD_OFFSET(reg);

        if (its->enabled)
            if (vgic_its_handle_cmds(d, its))
                MSGH("error handling ITS commands\n");

        spin_unlock(&its->vcmd_lock);

        return 1;
    case VREG64(GITS_CREADR):
        goto write_ignore_64;
    case VRANGE32(0x0098, 0x00FC):
        goto write_reserved;
    case VREG64(GITS_BASER0):
        if (!vgic_reg64_check_access(info->dabt))
            goto bad_width;

        spin_lock(&its->its_lock);

        if (its->enabled) {
            spin_unlock(&its->its_lock);
            MSGH(XENLOG_WARNING, "vGITS: tried to change BASER with the ITS enabled.\n");

            return 1;
        }

        reg = its->baser_dev;
        vreg_reg64_update(&reg, r, info);

        reg &= ~(GITS_BASER_RO_MASK | GITS_BASER_INDIRECT);
        reg |= (sizeof(dev_table_entry_t) - 1) << GITS_BASER_ENTRY_SIZE_SHIFT;
        reg |= GITS_BASER_TYPE_DEVICE << GITS_BASER_TYPE_SHIFT;
        sanitize_its_base_reg(&reg);

        if (reg & GITS_VALID_BIT) {
            its->max_devices = its_baser_nr_entries(reg);
            if (its->max_devices > BIT(its->devid_bits, UL))
                its->max_devices = BIT(its->devid_bits, UL);
        } else
            its->max_devices = 0;

        its->baser_dev = reg;
        spin_unlock(&its->its_lock);
        return 1;
    case VREG64(GITS_BASER1):
        if (!vgic_reg64_check_access(info->dabt))
            goto bad_width;

        spin_lock(&its->its_lock);

        if (its->enabled) {
            spin_unlock(&its->its_lock);
            MSGH("vGITS: tried to change BASER with the ITS enabled.\n");
            return 1;
        }

        reg = its->baser_coll;
        vreg_reg64_update(&reg, r, info);
        reg &= ~(GITS_BASER_RO_MASK | GITS_BASER_INDIRECT);
        reg |= (sizeof(coll_table_entry_t) - 1) << GITS_BASER_ENTRY_SIZE_SHIFT;
        reg |= GITS_BASER_TYPE_COLLECTION << GITS_BASER_TYPE_SHIFT;
        sanitize_its_base_reg(&reg);

        if (reg & GITS_VALID_BIT)
            its->max_collections = its_baser_nr_entries(reg);
        else
            its->max_collections = 0;
        its->baser_coll = reg;
        spin_unlock(&its->its_lock);
        return 1;
    case VRANGE64(GITS_BASER2, GITS_BASER7):
        goto write_ignore_64;
    case VRANGE32(0x0140, 0xBFFC):
        goto write_reserved;
    case VRANGE32(0xC000, 0xFFCC):
        goto write_impl_defined;
    case VRANGE32(0xFFD0, 0xFFE4):      /* IMPDEF identification registers */
        goto write_impl_defined;
    case VREG32(GITS_PIDR2):
        goto write_ignore_32;
    case VRANGE32(0xFFEC, 0xFFFC):      /* IMPDEF identification registers */
        goto write_impl_defined;
    default:
        MSGH("%pv: vGITS: unhandled write r%d offset %#04lx\n",
             v, info->dabt.reg, (unsigned long)info->gpa & 0xFFFF);
        return 0;
    }

    ASSERT_UNREACHABLE();
    return 1;

write_ignore_64:
    if (!vgic_reg64_check_access(info->dabt))
        goto bad_width;
    return 1;

write_ignore_32:
    if (info->dabt.size != DABT_WORD)
        goto bad_width;
    return 1;

write_impl_defined:
    MSGH("%pv: vGITS: WI on implementation defined register offset %#04lx\n",
         v, info->gpa & 0xFFFF);
    return 1;

write_reserved:
    MSGH("%pv: vGITS: WI on implementation defined register offset %#04lx\n",
         v, info->gpa & 0xFFFF);
    return 1;

bad_width:
    MSGH("vGITS: bad write width %d r%d offset %#08lx\n",
         info->dabt.size, info->dabt.reg, (unsigned long)info->gpa & 0xffff);

    return 0;
}

static const struct mmio_handler_ops vgic_its_mmio_handler = {
    .read  = vgic_v3_its_mmio_read,
    .write = vgic_v3_its_mmio_write,
};

static int vgic_v3_its_init_virtual(struct hypos *d, hpa_t guest_addr,
                                    unsigned int devid_bits,
                                    unsigned int evid_bits)
{
    struct virt_its *its;
    u64 base_attr;

    its = xzalloc(struct virt_its);
    if ( !its )
        return -ENOMEM;

    base_attr  = GIC_BASER_InnerShareable << GITS_BASER_SHAREABILITY_SHIFT;
    base_attr |= GIC_BASER_CACHE_SameAsInner << GITS_BASER_OUTER_CACHEABILITY_SHIFT;
    base_attr |= GIC_BASER_CACHE_RaWaWb << GITS_BASER_INNER_CACHEABILITY_SHIFT;

    its->cbaser     = base_attr;
    base_attr |= 0ULL << GITS_BASER_PAGE_SIZE_SHIFT;
    its->baser_dev  = GITS_BASER_TYPE_DEVICE << GITS_BASER_TYPE_SHIFT;
    its->baser_dev  |= (sizeof(dev_table_entry_t) - 1) <<
                      GITS_BASER_ENTRY_SIZE_SHIFT;
    its->baser_dev  |= base_attr;
    its->baser_coll = GITS_BASER_TYPE_COLLECTION << GITS_BASER_TYPE_SHIFT;
    its->baser_coll |= (sizeof(coll_table_entry_t) - 1) <<
                       GITS_BASER_ENTRY_SIZE_SHIFT;
    its->baser_coll |= base_attr;
    its->d = d;
    its->doorbell_address = guest_addr + ITS_DOORBELL_OFFSET;
    its->devid_bits = devid_bits;
    its->evid_bits  = evid_bits;
    spin_lock_init(&its->vcmd_lock);
    spin_lock_init(&its->its_lock);

    register_mmio_handler(d, &vgic_its_mmio_handler, guest_addr, KB(64), its);

    list_add_tail(&its->vits_list, &d->arch.vgic.vits_list);

    return 0;
}

unsigned int vgic_v3_its_count(const struct hypos *d)
{
    struct host_its *hw_its;
    unsigned int ret = 0;

    if (!is_hardware_hypos(d))
        return 0;

    list_for_each_entry(hw_its, &host_its_list, entry)
        ret++;

    return ret;
}

int vgic_v3_its_init_hypos(struct hypos *d)
{
    int ret;

    INIT_LIST_HEAD(&d->arch.vgic.vits_list);
    spin_lock_init(&d->arch.vgic.its_devices_lock);
    d->arch.vgic.its_devices = RB_ROOT;

    if (is_hardware_hypos(d)) {
        struct host_its *hw_its;

        list_for_each_entry(hw_its, &host_its_list, entry) {
            ret = vgic_v3_its_init_virtual(d, hw_its->addr,
                                           hw_its->devid_bits,
                                           hw_its->evid_bits);
            if (ret)
                return ret;
            else
                d->arch.vgic.has_its = true;
        }
    }

    return 0;
}

void vgic_v3_its_free_hypos(struct hypos *d)
{
    struct virt_its *pos, *temp;

    if (list_head_is_null(&d->arch.vgic.vits_list))
        return;

    list_for_each_entry_safe(pos, temp,
                             &d->arch.vgic.vits_list, vits_list) {
        list_del(&pos->vits_list);
        free(pos);
    }

    ASSERT(RB_EMPTY_ROOT(&d->arch.vgic.its_devices));
}

// --------------------------------------------------------------

#define GICV3_GICD_PIDR2  0x30
#define GICV3_GICR_PIDR2  GICV3_GICD_PIDR2

/*
 * GICD_CTLR default value:
 *      - No GICv2 compatibility => ARE = 1
 */
#define VGICD_CTLR_DEFAULT  (GICD_CTLR_ARE_NS)

static struct {
    bool enabled;
    hpa_t dbase;
    unsigned int nr_rdist_regions;
    const struct rdist_region *regions;
    unsigned int intid_bits;
} vgic_v3_hw;

void vgic_v3_setup_hw(hpa_t dbase,
                      unsigned int nr_rdist_regions,
                      const struct rdist_region *regions,
                      unsigned int intid_bits)
{
    vgic_v3_hw.enabled = true;
    vgic_v3_hw.dbase = dbase;
    vgic_v3_hw.nr_rdist_regions = nr_rdist_regions;
    vgic_v3_hw.regions = regions;
    vgic_v3_hw.intid_bits = intid_bits;
}

static struct vcpu *vgic_v3_irouter_to_vcpu(struct hypos *d, u64 irouter)
{
    unsigned int vcpu_id;

    if (irouter & GICD_IROUTER_SPI_MODE_ANY)
        return d->vcpu[0];

    vcpu_id = vaffinity_to_vcpuid(irouter);
    if (vcpu_id >= d->max_vcpus)
        return NULL;

    return d->vcpu[vcpu_id];
}

#define NR_BYTES_PER_IROUTER 8U

static u64 vgic_fetch_irouter(struct vgic_irq_rank *rank,
                              unsigned int offset)
{
    ASSERT(spin_is_locked(&rank->lock));

    offset /= NR_BYTES_PER_IROUTER;

    offset &= INTERRUPT_RANK_MASK;

    return vcpuid_to_vaffinity(read_atomic(&rank->vcpu[offset]));
}

static void vgic_store_irouter(struct hypos *d,
                               struct vgic_irq_rank *rank,
                               unsigned int offset, u64 irouter)
{
    struct vcpu *new_vcpu, *old_vcpu;
    unsigned int virq;

    virq = offset / NR_BYTES_PER_IROUTER;

    ASSERT(virq >= 32);

    offset = virq & INTERRUPT_RANK_MASK;

    new_vcpu = vgic_v3_irouter_to_vcpu(d, irouter);
    old_vcpu = d->vcpu[read_atomic(&rank->vcpu[offset])];

    if (!new_vcpu)
        return;

    if (new_vcpu != old_vcpu) {
        if (vgic_migrate_irq(old_vcpu, new_vcpu, virq))
            write_atomic(&rank->vcpu[offset], new_vcpu->vcpu_id);
    }
}

static int __vgic_v3_rdistr_rd_mmio_read(struct vcpu *v,
                                         mmio_info_t *info,
                                         u32 gicr_reg,
                                         register_t *r)
{
    struct hsr_dabt dabt = info->dabt;

    switch (gicr_reg) {
    case VREG32(GICR_CTLR):
    {
        unsigned long flags;

        if (!v->hypos->arch.vgic.has_its)
            goto read_as_zero_32;
        if (dabt.size != DABT_WORD)
            goto bad_width;

        spin_lock_irqsave(&v->arch.vgic.lock, flags);
        *r = vreg_reg32_extract(!!(v->arch.vgic.flags
                                & VGIC_V3_LPIS_ENABLED),
                                info);
        spin_unlock_irqrestore(&v->arch.vgic.lock, flags);
        return 1;
    }
    case VREG32(GICR_IIDR):
        if (dabt.size != DABT_WORD)
            goto bad_width;
        *r = vreg_reg32_extract(GICV3_GICR_IIDR_VAL, info);
        return 1;
    case VREG64(GICR_TYPER):
    {
        u64 typer, aff;
        u64 vmpidr = v->arch.vmpidr;

        if ( !vgic_reg64_check_access(dabt) ) goto bad_width;
        aff = (MPIDR_AFFINITY_LEVEL(vmpidr, 3) << 56 |
               MPIDR_AFFINITY_LEVEL(vmpidr, 2) << 48 |
               MPIDR_AFFINITY_LEVEL(vmpidr, 1) << 40 |
               MPIDR_AFFINITY_LEVEL(vmpidr, 0) << 32);

        typer = aff;
        typer |= v->vcpu_id << GICR_TYPER_PROC_NUM_SHIFT;

        if (v->arch.vgic.flags & VGIC_V3_RDIST_LAST)
            typer |= GICR_TYPER_LAST;

        if (v->hypos->arch.vgic.has_its)
            typer |= GICR_TYPER_PLPIS;

        *r = vreg_reg64_extract(typer, info);

        return 1;
    }
    case VREG32(GICR_STATUSR):
        goto read_as_zero_32;
    case VREG32(GICR_WAKER):
        goto read_as_zero_32;
    case 0x0018:
        goto read_reserved;
    case 0x0020:
        goto read_impl_defined;
    case VREG64(GICR_SETLPIR):
        goto read_unknown;
    case VREG64(GICR_CLRLPIR):
        goto read_unknown;
    case 0x0050:
        goto read_reserved;
    case VREG64(GICR_PROPBASER):
        if (!v->hypos->arch.vgic.has_its)
            goto read_as_zero_64;
        if (!vgic_reg64_check_access(dabt))
            goto bad_width;

        vgic_lock(v);
        *r = vreg_reg64_extract(v->hypos->arch.vgic.rdist_propbase,
                info);
        vgic_unlock(v);
        return 1;
    case VREG64(GICR_PENDBASER):
    {
        u64 val;

        if (!v->hypos->arch.vgic.has_its)
            goto read_as_zero_64;
        if (!vgic_reg64_check_access(dabt))
            goto bad_width;

        val = read_atomic(&v->arch.vgic.rdist_pendbase);
        val &= ~GICR_PENDBASER_PTZ;
        *r = vreg_reg64_extract(val, info);
        return 1;
    }
    case 0x0080:
        goto read_reserved;
    case VREG64(GICR_INVLPIR):
        goto read_unknown;
    case 0x00A8:
        goto read_reserved;
    case VREG64(GICR_INVALLR):
        goto read_unknown;
    case 0x00B8:
        goto read_reserved;
    case VREG32(GICR_SYNCR):
        if (dabt.size != DABT_WORD)
            goto bad_width;
        *r = vreg_reg32_extract(GICR_SYNCR_NOT_BUSY, info);
        return 1;
    case 0x00C8:
        goto read_reserved;
    case VREG64(0x0100):
        goto read_impl_defined;
    case 0x0108:
        goto read_reserved;
    case VREG64(0x0110):
        goto read_impl_defined;
    case 0x0118 ... 0xBFFC:
        goto read_reserved;
    case 0xC000 ... 0xFFCC:
        goto read_impl_defined;
    case 0xFFD0 ... 0xFFE4:
        goto read_impl_defined;
    case VREG32(GICR_PIDR2):
        if (dabt.size != DABT_WORD)
            goto bad_width;
        *r = vreg_reg32_extract(GICV3_GICR_PIDR2, info);
        return 1;
    case 0xFFEC ... 0xFFFC:
         goto read_impl_defined;
    default:
        MSGH("%pv: vGICR: unhandled read r%d offset %#08x\n",
             v, dabt.reg, gicr_reg);
        goto read_as_zero;
    }
bad_width:
    MSGH("%pv vGICR: bad read width %d r%d offset %#08x\n",
         v, dabt.size, dabt.reg, gicr_reg);
    return 0;

read_as_zero_64:
    if (!vgic_reg64_check_access(dabt))
        goto bad_width;
    *r = 0;
    return 1;

read_as_zero_32:
    if (dabt.size != DABT_WORD)
        goto bad_width;
    *r = 0;
    return 1;

read_as_zero:
    *r = 0;
    return 1;

read_impl_defined:
    MSGH("%pv: vGICR: RAZ on implementation defined register offset %#08x\n",
         v, gicr_reg);
    *r = 0;
    return 1;

read_reserved:
    MSGH("%pv: vGICR: RAZ on reserved register offset %#08x\n",
         v, gicr_reg);
    *r = 0;
    return 1;

read_unknown:
    *r = vreg_reg64_extract(0xdeadbeafdeadbeafULL, info);
    return 1;
}

static u64 vgic_sanitise_field(u64 reg, u64 field_mask,
                               int field_shift,
                               u64 (*sanitise_fn)(u64 field))
{
    u64 field = (reg & field_mask) >> field_shift;

    field = sanitise_fn(field) << field_shift;

    return (reg & ~field_mask) | field;
}

/* We want to avoid outer shareable. */
static u64 vgic_sanitise_shareability(u64 field)
{
    switch (field) {
    case GIC_BASER_OuterShareable:
        return GIC_BASER_InnerShareable;
    default:
        return field;
    }
}

/* Avoid any inner non-cacheable mapping. */
static u64 vgic_sanitise_inner_cacheability(u64 field)
{
    switch (field) {
    case GIC_BASER_CACHE_nCnB:
    case GIC_BASER_CACHE_nC:
        return GIC_BASER_CACHE_RaWb;
    default:
        return field;
    }
}

/* Non-cacheable or same-as-inner are OK. */
static u64 vgic_sanitise_outer_cacheability(u64 field)
{
    switch (field) {
    case GIC_BASER_CACHE_SameAsInner:
    case GIC_BASER_CACHE_nC:
        return field;
    default:
        return GIC_BASER_CACHE_nC;
    }
}

static u64 sanitize_propbaser(u64 reg)
{
    reg = vgic_sanitise_field(reg, GICR_PROPBASER_SHAREABILITY_MASK,
                              GICR_PROPBASER_SHAREABILITY_SHIFT,
                              vgic_sanitise_shareability);
    reg = vgic_sanitise_field(reg, GICR_PROPBASER_INNER_CACHEABILITY_MASK,
                              GICR_PROPBASER_INNER_CACHEABILITY_SHIFT,
                              vgic_sanitise_inner_cacheability);
    reg = vgic_sanitise_field(reg, GICR_PROPBASER_OUTER_CACHEABILITY_MASK,
                              GICR_PROPBASER_OUTER_CACHEABILITY_SHIFT,
                              vgic_sanitise_outer_cacheability);

    reg &= ~GICR_PROPBASER_RES0_MASK;

    return reg;
}

static u64 sanitize_pendbaser(u64 reg)
{
    reg = vgic_sanitise_field(reg, GICR_PENDBASER_SHAREABILITY_MASK,
                              GICR_PENDBASER_SHAREABILITY_SHIFT,
                              vgic_sanitise_shareability);
    reg = vgic_sanitise_field(reg, GICR_PENDBASER_INNER_CACHEABILITY_MASK,
                              GICR_PENDBASER_INNER_CACHEABILITY_SHIFT,
                              vgic_sanitise_inner_cacheability);
    reg = vgic_sanitise_field(reg, GICR_PENDBASER_OUTER_CACHEABILITY_MASK,
                              GICR_PENDBASER_OUTER_CACHEABILITY_SHIFT,
                              vgic_sanitise_outer_cacheability);

    reg &= ~GICR_PENDBASER_RES0_MASK;

    return reg;
}

static void vgic_vcpu_enable_lpis(struct vcpu *v)
{
    u64 reg = v->hypos->arch.vgic.rdist_propbase;
    unsigned int nr_lpis = BIT((reg & 0x1F) + 1, UL);

    ASSERT(spin_is_locked(&v->hypos->arch.vgic.lock));

    if (nr_lpis < LPI_OFFSET)
        nr_lpis = 0;
    else
        nr_lpis -= LPI_OFFSET;

    if (!v->hypos->arch.vgic.rdists_enabled) {
        v->hypos->arch.vgic.nr_lpis = nr_lpis;
        smp_mb();
        v->hypos->arch.vgic.rdists_enabled = true;
        smp_mb();
    }

    v->arch.vgic.flags |= VGIC_V3_LPIS_ENABLED;
}

static int __vgic_v3_rdistr_rd_mmio_write(struct vcpu *v,
                                          mmio_info_t *info,
                                          u32 gicr_reg,
                                          register_t r)
{
    struct hsr_dabt dabt = info->dabt;
    u64 reg;

    switch (gicr_reg) {
    case VREG32(GICR_CTLR):
    {
        unsigned long flags;

        if (!v->hypos->arch.vgic.has_its)
            goto write_ignore_32;
        if (dabt.size != DABT_WORD)
            goto bad_width;

        vgic_lock(v);
        spin_lock_irqsave(&v->arch.vgic.lock, flags);

        if ((r & GICR_CTLR_ENABLE_LPIS) &&
             !(v->arch.vgic.flags & VGIC_V3_LPIS_ENABLED))
            vgic_vcpu_enable_lpis(v);

        spin_unlock_irqrestore(&v->arch.vgic.lock, flags);
        vgic_unlock(v);

        return 1;
    }
    case VREG32(GICR_IIDR):
        goto write_ignore_32;
    case VREG64(GICR_TYPER):
        goto write_ignore_64;
    case VREG32(GICR_STATUSR):
        goto write_ignore_32;
    case VREG32(GICR_WAKER):
        goto write_ignore_32;
    case 0x0018:
        goto write_reserved;
    case 0x0020:
        goto write_impl_defined;
    case VREG64(GICR_SETLPIR):
        goto write_ignore_64;
    case VREG64(GICR_CLRLPIR):
        goto write_ignore_64;
    case 0x0050:
        goto write_reserved;
    case VREG64(GICR_PROPBASER):
        if (!v->hypos->arch.vgic.has_its)
            goto write_ignore_64;
        if (!vgic_reg64_check_access(dabt))
            goto bad_width;

        vgic_lock(v);

        if (!(v->hypos->arch.vgic.rdists_enabled)) {
            reg = v->hypos->arch.vgic.rdist_propbase;
            vreg_reg64_update(&reg, r, info);
            reg = sanitize_propbaser(reg);
            v->hypos->arch.vgic.rdist_propbase = reg;
        }

        vgic_unlock(v);

        return 1;
    case VREG64(GICR_PENDBASER):
    {
        unsigned long flags;

        if (!v->hypos->arch.vgic.has_its)
            goto write_ignore_64;
        if (!vgic_reg64_check_access(dabt))
            goto bad_width;

        spin_lock_irqsave(&v->arch.vgic.lock, flags);

        if (!(v->arch.vgic.flags & VGIC_V3_LPIS_ENABLED)) {
            reg = read_atomic(&v->arch.vgic.rdist_pendbase);
            vreg_reg64_update(&reg, r, info);
            reg = sanitize_pendbaser(reg);
            write_atomic(&v->arch.vgic.rdist_pendbase, reg);
        }

        spin_unlock_irqrestore(&v->arch.vgic.lock, flags);

        return 1;
    }
    case 0x0080:
        goto write_reserved;
    case VREG64(GICR_INVLPIR):
        goto write_ignore_64;
    case 0x00A8:
        goto write_reserved;

    case VREG64(GICR_INVALLR):
        goto write_ignore_64;
    case 0x00B8:
        goto write_reserved;
    case VREG32(GICR_SYNCR):
        goto write_ignore_32;
    case 0x00C8:
        goto write_reserved;
    case VREG64(0x0100):
        goto write_impl_defined;
    case 0x0108:
        goto write_reserved;
    case VREG64(0x0110):
        goto write_impl_defined;
    case 0x0118 ... 0xBFFC:
        goto write_reserved;
    case 0xC000 ... 0xFFCC:
        goto write_impl_defined;
    case 0xFFD0 ... 0xFFE4:
        goto write_impl_defined;
    case VREG32(GICR_PIDR2):
        goto write_ignore_32;
    case 0xFFEC ... 0xFFFC:
        goto write_impl_defined;
    default:
        MSGH("%pv: vGICR: unhandled write r%d offset %#08x\n",
             v, dabt.reg, gicr_reg);
        goto write_ignore;
    }
bad_width:
    MSGH("%pv: vGICR: bad write width %d r%d=%016lx offset %#08x\n",
         v, dabt.size, dabt.reg, r, gicr_reg);
    return 0;

write_ignore_64:
    if (vgic_reg64_check_access(dabt))
        goto bad_width;
    return 1;

write_ignore_32:
    if (dabt.size != DABT_WORD)
        goto bad_width;
    return 1;

write_ignore:
    return 1;

write_impl_defined:
    MSGH("%pv: vGICR: WI on implementation defined register offset %#08x\n",
         v, gicr_reg);
    return 1;

write_reserved:
    MSGH("%pv: vGICR: WI on reserved register offset %#08x\n",
         v, gicr_reg);
    return 1;
}

static int __vgic_v3_distr_common_mmio_read(const char *name, struct vcpu *v,
                                            mmio_info_t *info, u32 reg,
                                            register_t *r)
{
    struct hsr_dabt dabt = info->dabt;
    struct vgic_irq_rank *rank;
    unsigned long flags;

    switch (reg) {
    case VRANGE32(GICD_IGROUPR, GICD_IGROUPRN):
    case VRANGE32(GICD_IGRPMODR, GICD_IGRPMODRN):
        /* We do not implement security extensions for guests, read zero */
        if (dabt.size != DABT_WORD)
            goto bad_width;
        goto read_as_zero;

    case VRANGE32(GICD_ISENABLER, GICD_ISENABLERN):
        if (dabt.size != DABT_WORD)
            goto bad_width;
        rank = vgic_rank_offset(v, 1, reg - GICD_ISENABLER, DABT_WORD);
        if (rank == NULL)
            goto read_as_zero;
        vgic_lock_rank(v, rank, flags);
        *r = vreg_reg32_extract(rank->ienable, info);
        vgic_unlock_rank(v, rank, flags);
        return 1;
    case VRANGE32(GICD_ICENABLER, GICD_ICENABLERN):
        if (dabt.size != DABT_WORD)
            goto bad_width;
        rank = vgic_rank_offset(v, 1, reg - GICD_ICENABLER, DABT_WORD);
        if (rank == NULL)
            goto read_as_zero;
        vgic_lock_rank(v, rank, flags);
        *r = vreg_reg32_extract(rank->ienable, info);
        vgic_unlock_rank(v, rank, flags);
        return 1;
    case VRANGE32(GICD_ISPENDR, GICD_ISPENDRN):
    case VRANGE32(GICD_ICPENDR, GICD_ICPENDR):
        goto read_as_zero;
    case VRANGE32(GICD_ISACTIVER, GICD_ISACTIVERN):
    case VRANGE32(GICD_ICACTIVER, GICD_ICACTIVERN):
        goto read_as_zero;
    case VRANGE32(GICD_IPRIORITYR, GICD_IPRIORITYRN):
    {
        u32 ipriorityr;
        uint8_t rank_index;

        if (dabt.size != DABT_BYTE && dabt.size != DABT_WORD)
            goto bad_width;
        rank = vgic_rank_offset(v, 8, reg - GICD_IPRIORITYR, DABT_WORD);
        if (rank == NULL)
            goto read_as_zero;
        rank_index = REG_RANK_INDEX(8, reg - GICD_IPRIORITYR, DABT_WORD);

        vgic_lock_rank(v, rank, flags);
        ipriorityr = ACCESS_ONCE(rank->ipriorityr[rank_index]);
        vgic_unlock_rank(v, rank, flags);

        *r = vreg_reg32_extract(ipriorityr, info);

        return 1;
    }

    case VRANGE32(GICD_ICFGR, GICD_ICFGRN):
    {
        u32 icfgr;

        if (dabt.size != DABT_WORD)
            goto bad_width;
        rank = vgic_rank_offset(v, 2, reg - GICD_ICFGR, DABT_WORD);
        if (rank == NULL)
            goto read_as_zero;
        vgic_lock_rank(v, rank, flags);
        icfgr = rank->icfg[REG_RANK_INDEX(2, reg - GICD_ICFGR, DABT_WORD)];
        vgic_unlock_rank(v, rank, flags);

        *r = vreg_reg32_extract(icfgr, info);

        return 1;
    }

    default:
        MSGH("%pv: %s: unhandled read r%d offset %#08x\n",
             v, name, dabt.reg, reg);
        return 0;
    }

bad_width:
    MSGH("%pv: %s: bad read width %d r%d offset %#08x\n",
         v, name, dabt.size, dabt.reg, reg);
    return 0;

read_as_zero:
    *r = 0;
    return 1;
}

static int __vgic_v3_distr_common_mmio_write(const char *name,
                                             struct vcpu *v,
                                             mmio_info_t *info,
                                             u32 reg,
                                             register_t r)
{
    struct hsr_dabt dabt = info->dabt;
    struct vgic_irq_rank *rank;
    u32 tr;
    unsigned long flags;

    switch (reg) {
    case VRANGE32(GICD_IGROUPR, GICD_IGROUPRN):
    case VRANGE32(GICD_IGRPMODR, GICD_IGRPMODRN):
        goto write_ignore_32;

    case VRANGE32(GICD_ISENABLER, GICD_ISENABLERN):
        if (dabt.size != DABT_WORD)
            goto bad_width;
        rank = vgic_rank_offset(v, 1, reg - GICD_ISENABLER, DABT_WORD);
        if (rank == NULL)
            goto write_ignore;
        vgic_lock_rank(v, rank, flags);
        tr = rank->ienable;
        vreg_reg32_setbits(&rank->ienable, r, info);
        vgic_enable_irqs(v, (rank->ienable) & (~tr), rank->index);
        vgic_unlock_rank(v, rank, flags);
        return 1;

    case VRANGE32(GICD_ICENABLER, GICD_ICENABLERN):
        if (dabt.size != DABT_WORD)
            goto bad_width;

        rank = vgic_rank_offset(v, 1, reg - GICD_ICENABLER, DABT_WORD);
        if (rank == NULL)
            goto write_ignore;

        vgic_lock_rank(v, rank, flags);
        tr = rank->ienable;
        vreg_reg32_clearbits(&rank->ienable, r, info);
        vgic_disable_irqs(v, (~rank->ienable) & tr, rank->index);
        vgic_unlock_rank(v, rank, flags);
        return 1;

    case VRANGE32(GICD_ISPENDR, GICD_ISPENDRN):
        if (dabt.size != DABT_WORD)
            goto bad_width;
        rank = vgic_rank_offset(v, 1, reg - GICD_ISPENDR, DABT_WORD);
        if (rank == NULL)
            goto write_ignore;

        vgic_set_irqs_pending(v, r, rank->index);

        return 1;

    case VRANGE32(GICD_ICPENDR, GICD_ICPENDRN):
        if (dabt.size != DABT_WORD)
            goto bad_width;
        rank = vgic_rank_offset(v, 1, reg - GICD_ICPENDR, DABT_WORD);
        if (rank == NULL)
            goto write_ignore;

        vgic_check_inflight_irqs_pending(v->hypos, v, rank->index, r);

        goto write_ignore;

    case VRANGE32(GICD_ISACTIVER, GICD_ISACTIVERN):
        if (dabt.size != DABT_WORD)
            goto bad_width;
        MSGH("%pv: %s: unhandled word write %#016lx to ISACTIVER%d\n",
             v, name, r, reg - GICD_ISACTIVER);
        return 0;

    case VRANGE32(GICD_ICACTIVER, GICD_ICACTIVERN):
        MSGH("%pv: %s: unhandled word write %#016lx to ICACTIVER%d\n",
             v, name, r, reg - GICD_ICACTIVER);
        goto write_ignore_32;

    case VRANGE32(GICD_IPRIORITYR, GICD_IPRIORITYRN):
    {
        u32 *ipriorityr, priority;

        if (dabt.size != DABT_BYTE && dabt.size != DABT_WORD)
            goto bad_width;
        rank = vgic_rank_offset(v, 8, reg - GICD_IPRIORITYR, DABT_WORD);
        if (rank == NULL)
            goto write_ignore;
        vgic_lock_rank(v, rank, flags);
        ipriorityr = &rank->ipriorityr[REG_RANK_INDEX(8, reg - GICD_IPRIORITYR,
                                                      DABT_WORD)];
        priority = ACCESS_ONCE(*ipriorityr);
        vreg_reg32_update(&priority, r, info);
        ACCESS_ONCE(*ipriorityr) = priority;
        vgic_unlock_rank(v, rank, flags);
        return 1;
    }

    case VREG32(GICD_ICFGR):
        goto write_ignore_32;

    case VRANGE32(GICD_ICFGR + 4, GICD_ICFGRN):
        if (dabt.size != DABT_WORD)
            goto bad_width;

        rank = vgic_rank_offset(v, 2, reg - GICD_ICFGR, DABT_WORD);
        if (rank == NULL)
            goto write_ignore;

        vgic_lock_rank(v, rank, flags);
        vreg_reg32_update(&rank->icfg[REG_RANK_INDEX(2, reg - GICD_ICFGR,
                                                     DABT_WORD)],
                          r, info);
        vgic_unlock_rank(v, rank, flags);
        return 1;

    default:
        MSGH("%pv: %s: unhandled write r%d=%016lx offset %#08x\n",
             v, name, dabt.reg, r, reg);
        return 0;
    }

bad_width:
    MSGH("%pv: %s: bad write width %d r%d=%016lx offset %#08x\n",
         v, name, dabt.size, dabt.reg, r, reg);
    return 0;

write_ignore_32:
    if ( dabt.size != DABT_WORD ) goto bad_width;
write_ignore:
    return 1;
}

static int vgic_v3_rdistr_sgi_mmio_read(struct vcpu *v, mmio_info_t *info,
                                        u32 gicr_reg, register_t *r)
{
    struct hsr_dabt dabt = info->dabt;

    switch (gicr_reg) {
    case VREG32(GICR_IGROUPR0):
    case VREG32(GICR_ISENABLER0):
    case VREG32(GICR_ICENABLER0):
    case VREG32(GICR_ISACTIVER0):
    case VREG32(GICR_ICACTIVER0):
    case VRANGE32(GICR_IPRIORITYR0, GICR_IPRIORITYR7):
    case VRANGE32(GICR_ICFGR0, GICR_ICFGR1):
        return __vgic_v3_distr_common_mmio_read("vGICR: SGI", v, info,
                                                gicr_reg, r);

    case VREG32(GICR_ISPENDR0):
    case VREG32(GICR_ICPENDR0):
        goto read_as_zero;

    case VREG32(GICR_IGRPMODR0):
        goto read_as_zero_32;

    case VREG32(GICR_NSACR):
        goto read_as_zero_32;

    case 0x0E04 ... 0xBFFC:
        goto read_reserved;

    case 0xC000 ... 0xFFCC:
        goto read_impl_defined;

    case 0xFFD0 ... 0xFFFC:
        goto read_reserved;

    default:
        MSGH("%pv: vGICR: SGI: unhandled read r%d offset %#08x\n",
             v, dabt.reg, gicr_reg);
        goto read_as_zero;
    }
bad_width:
    MSGH("%pv: vGICR: SGI: bad read width %d r%d offset %#08x\n",
         v, dabt.size, dabt.reg, gicr_reg);
    return 0;

read_as_zero_32:
    if (dabt.size != DABT_WORD)
        goto bad_width;
read_as_zero:
    *r = 0;
    return 1;

read_impl_defined:
    MSGH("%pv: vGICR: SGI: RAZ on implementation defined register offset %#08x\n",
         v, gicr_reg);
    *r = 0;
    return 1;

read_reserved:
    MSGH("%pv: vGICR: SGI: RAZ on reserved register offset %#08x\n",
         v, gicr_reg);
    *r = 0;
    return 1;

}

static int vgic_v3_rdistr_sgi_mmio_write(struct vcpu *v, mmio_info_t *info,
                                         u32 gicr_reg, register_t r)
{
    struct hsr_dabt dabt = info->dabt;

    switch (gicr_reg) {
    case VREG32(GICR_IGROUPR0):
    case VREG32(GICR_ISENABLER0):
    case VREG32(GICR_ICENABLER0):
    case VREG32(GICR_ISACTIVER0):
    case VREG32(GICR_ICACTIVER0):
    case VREG32(GICR_ICFGR1):
    case VRANGE32(GICR_IPRIORITYR0, GICR_IPRIORITYR7):
    case VREG32(GICR_ISPENDR0):
        return __vgic_v3_distr_common_mmio_write("vGICR: SGI", v,
                                                 info, gicr_reg, r);

    case VREG32(GICR_ICPENDR0):
        return __vgic_v3_distr_common_mmio_write("vGICR: SGI", v,
                                                 info, gicr_reg, r);

    case VREG32(GICR_IGRPMODR0):
        goto write_ignore_32;


    case VREG32(GICR_NSACR):
        goto write_ignore_32;

    default:
        MSGH("%pv: vGICR: SGI: unhandled write r%d offset %#08x\n",
             v, dabt.reg, gicr_reg);
        goto write_ignore;
    }

bad_width:
    MSGH("%pv: vGICR: SGI: bad write width %d r%d=%016lx offset %#08x\n",
         v, dabt.size, dabt.reg, r, gicr_reg);
    return 0;

write_ignore_32:
    if (dabt.size != DABT_WORD)
        goto bad_width;
    return 1;

write_ignore:
    return 1;
}

static struct vcpu *get_vcpu_from_rdist(struct hypos *d,
    const struct vgic_rdist_region *region,
    hpa_t gpa, u32 *offset)
{
    struct vcpu *v;
    unsigned int vcpu_id;

    vcpu_id = region->first_cpu + ((gpa - region->base) / GICV3_GICR_SIZE);
    if (unlikely(vcpu_id >= d->max_vcpus))
        return NULL;

    v = d->vcpu[vcpu_id];

    *offset = gpa - v->arch.vgic.rdist_base;

    return v;
}

static int vgic_v3_rdistr_mmio_read(struct vcpu *v, mmio_info_t *info,
                                    register_t *r, void *priv)
{
    u32 offset;
    const struct vgic_rdist_region *region = priv;

    perfc_incr(vgicr_reads);

    v = get_vcpu_from_rdist(v->hypos, region, info->gpa, &offset);
    if (unlikely(!v))
        return 0;

    if (offset < KB(64))
        return __vgic_v3_rdistr_rd_mmio_read(v, info, offset, r);
    else  if ((offset >= KB(64)) && (offset < 2 * KB(64)))
        return vgic_v3_rdistr_sgi_mmio_read(v, info, (offset - KB(64)), r);
    else
        MSGH("%pv: vGICR: unknown gpa read address %016lx\n",
             v, info->gpa);

    return 0;
}

static int vgic_v3_rdistr_mmio_write(struct vcpu *v, mmio_info_t *info,
                                     register_t r, void *priv)
{
    u32 offset;
    const struct vgic_rdist_region *region = priv;

    v = get_vcpu_from_rdist(v->hypos, region, info->gpa, &offset);
    if (unlikely(!v))
        return 0;

    if (offset < KB(64))
        return __vgic_v3_rdistr_rd_mmio_write(v, info, offset, r);
    else  if ((offset >= KB(64)) && (offset < 2 * KB(64)))
        return vgic_v3_rdistr_sgi_mmio_write(v, info, (offset - KB(64)), r);
    else
        MSGH("%pv: vGICR: unknown gpa write address %016lx\n",
             v, info->gpa);

    return 0;
}

static int vgic_v3_distr_mmio_read(struct vcpu *v, mmio_info_t *info,
                                   register_t *r, void *priv)
{
    struct hsr_dabt dabt = info->dabt;
    struct vgic_irq_rank *rank;
    unsigned long flags;
    int gicd_reg = (int)(info->gpa - v->hypos->arch.vgic.dbase);

    switch (gicd_reg) {
    case VREG32(GICD_CTLR):
        if (dabt.size != DABT_WORD)
            goto bad_width;
        vgic_lock(v);
        *r = vreg_reg32_extract(v->hypos->arch.vgic.ctlr, info);
        vgic_unlock(v);
        return 1;
    case VREG32(GICD_TYPER):
    {
        unsigned int ncpus = min_t(unsigned int, v->hypos->max_vcpus, 8);
        u32 typer;

        if (dabt.size != DABT_WORD)
            goto bad_width;

        typer = ((ncpus - 1) << GICD_TYPE_CPUS_SHIFT |
                 DIV_ROUND_UP(v->hypos->arch.vgic.nr_spis, 32));

        if (v->hypos->arch.vgic.has_its)
            typer |= GICD_TYPE_LPIS;

        typer |= (v->hypos->arch.vgic.intid_bits - 1) << GICD_TYPE_ID_BITS_SHIFT;

        *r = vreg_reg32_extract(typer, info);

        return 1;
    }
    case VREG32(GICD_IIDR):
        if ( dabt.size != DABT_WORD ) goto bad_width;
        *r = vreg_reg32_extract(GICV3_GICD_IIDR_VAL, info);
        return 1;
    case VREG32(0x000C):
        goto read_reserved;

    case VREG32(GICD_STATUSR):
        goto read_as_zero_32;

    case VRANGE32(0x0014, 0x001C):
        goto read_reserved;

    case VRANGE32(0x0020, 0x003C):
        goto read_impl_defined;

    case VREG32(GICD_SETSPI_NSR):
        goto read_reserved;

    case VREG32(0x0044):
        goto read_reserved;

    case VREG32(GICD_CLRSPI_NSR):
        goto read_reserved;

    case VREG32(0x004C):
        goto read_reserved;

    case VREG32(GICD_SETSPI_SR):
        goto read_reserved;

    case VREG32(0x0054):
        goto read_reserved;

    case VREG32(GICD_CLRSPI_SR):
        goto read_reserved;

    case VRANGE32(0x005C, 0x007C):
        goto read_reserved;
    case VRANGE32(GICD_IGROUPR, GICD_IGROUPRN):
    case VRANGE32(GICD_ISENABLER, GICD_ISENABLERN):
    case VRANGE32(GICD_ICENABLER, GICD_ICENABLERN):
    case VRANGE32(GICD_ISPENDR, GICD_ISPENDRN):
    case VRANGE32(GICD_ICPENDR, GICD_ICPENDRN):
    case VRANGE32(GICD_ISACTIVER, GICD_ISACTIVERN):
    case VRANGE32(GICD_ICACTIVER, GICD_ICACTIVERN):
    case VRANGE32(GICD_IPRIORITYR, GICD_IPRIORITYRN):
    case VRANGE32(GICD_ICFGR, GICD_ICFGRN):
    case VRANGE32(GICD_IGRPMODR, GICD_IGRPMODRN):
        return __vgic_v3_distr_common_mmio_read("vGICD", v, info, gicd_reg, r);
    case VRANGE32(GICD_NSACR, GICD_NSACRN):
        goto read_as_zero_32;
    case VREG32(GICD_SGIR):
        goto read_as_zero_32;
    case VRANGE32(GICD_CPENDSGIR, GICD_CPENDSGIRN):
        goto read_as_zero_32;
    case VRANGE32(GICD_SPENDSGIR, GICD_SPENDSGIRN):
        goto read_as_zero_32;
    case VRANGE32(0x0F30, 0x60FC):
        goto read_reserved;
    case VRANGE64(GICD_IROUTER32, GICD_IROUTER1019):
    {
        u64 irouter;

        if (!vgic_reg64_check_access(dabt))
            goto bad_width;
        rank = vgic_rank_offset(v, 64, gicd_reg - GICD_IROUTER,
                                DABT_DOUBLE_WORD);
        if (rank == NULL)
            goto read_as_zero;

        vgic_lock_rank(v, rank, flags);
        irouter = vgic_fetch_irouter(rank, gicd_reg - GICD_IROUTER);
        vgic_unlock_rank(v, rank, flags);

        *r = vreg_reg64_extract(irouter, info);

        return 1;
    }
    case VRANGE32(0x7FE0, 0xBFFC):
        goto read_reserved;
    case VRANGE32(0xC000, 0xFFCC):
        goto read_impl_defined;
    case VRANGE32(0xFFD0, 0xFFE4):
        goto read_impl_defined;
    case VREG32(GICD_PIDR2):
        if (dabt.size != DABT_WORD)
            goto bad_width;
        *r = vreg_reg32_extract(GICV3_GICD_PIDR2, info);
        return 1;
    case VRANGE32(0xFFEC, 0xFFFC):
        goto read_impl_defined;
    default:
        MSGH("%pv: vGICD: unhandled read r%d offset %#08x\n",
             v, dabt.reg, gicd_reg);
        goto read_as_zero;
    }

bad_width:
    MSGH("%pv: vGICD: bad read width %d r%d offset %#08x\n",
         v, dabt.size, dabt.reg, gicd_reg);
    return 0;

read_as_zero_32:
    if ( dabt.size != DABT_WORD ) goto bad_width;
    *r = 0;
    return 1;

read_as_zero:
    *r = 0;
    return 1;

read_impl_defined:
    MSGH("%pv: vGICD: RAZ on implementation defined register offset %#08x\n",
         v, gicd_reg);
    *r = 0;
    return 1;

read_reserved:
    MSGH("%pv: vGICD: RAZ on reserved register offset %#08x\n",
         v, gicd_reg);
    *r = 0;
    return 1;
}

static int vgic_v3_distr_mmio_write(struct vcpu *v, mmio_info_t *info,
                                    register_t r, void *priv)
{
    struct hsr_dabt dabt = info->dabt;
    struct vgic_irq_rank *rank;
    unsigned long flags;
    int gicd_reg = (int)(info->gpa - v->hypos->arch.vgic.dbase);

    switch (gicd_reg) {
    case VREG32(GICD_CTLR):
    {
        u32 ctlr = 0;

        if (dabt.size != DABT_WORD)
            goto bad_width;

        vgic_lock(v);

        vreg_reg32_update(&ctlr, r, info);

        if (ctlr & GICD_CTLR_ENABLE_G1A)
            v->hypos->arch.vgic.ctlr |= GICD_CTLR_ENABLE_G1A;
        else
            v->hypos->arch.vgic.ctlr &= ~GICD_CTLR_ENABLE_G1A;
        vgic_unlock(v);

        return 1;
    }
    case VREG32(GICD_TYPER):
        goto write_ignore_32;
    case VREG32(GICD_IIDR):
        goto write_ignore_32;
    case VREG32(0x000C):
        goto write_reserved;
    case VREG32(GICD_STATUSR):
        goto write_ignore_32;
    case VRANGE32(0x0014, 0x001C):
        goto write_reserved;
    case VRANGE32(0x0020, 0x003C):
        goto write_impl_defined;
    case VREG32(GICD_SETSPI_NSR):
        goto write_reserved;
    case VREG32(0x0044):
        goto write_reserved;
    case VREG32(GICD_CLRSPI_NSR):
        goto write_reserved;
    case VREG32(0x004C):
        goto write_reserved;
    case VREG32(GICD_SETSPI_SR):
        goto write_reserved;
    case VREG32(0x0054):
        goto write_reserved;
    case VREG32(GICD_CLRSPI_SR):
        goto write_reserved;
    case VRANGE32(0x005C, 0x007C):
        goto write_reserved;
    case VRANGE32(GICD_IGROUPR, GICD_IGROUPRN):
    case VRANGE32(GICD_ISENABLER, GICD_ISENABLERN):
    case VRANGE32(GICD_ICENABLER, GICD_ICENABLERN):
    case VRANGE32(GICD_ISPENDR, GICD_ISPENDRN):
    case VRANGE32(GICD_ICPENDR, GICD_ICPENDRN):
    case VRANGE32(GICD_ISACTIVER, GICD_ISACTIVERN):
    case VRANGE32(GICD_ICACTIVER, GICD_ICACTIVERN):
    case VRANGE32(GICD_IPRIORITYR, GICD_IPRIORITYRN):
    case VRANGE32(GICD_ICFGR, GICD_ICFGRN):
    case VRANGE32(GICD_IGRPMODR, GICD_IGRPMODRN):
        return __vgic_v3_distr_common_mmio_write("vGICD", v, info,
                                                 gicd_reg, r);
    case VRANGE32(GICD_NSACR, GICD_NSACRN):
        goto write_ignore_32;
    case VREG32(GICD_SGIR):
        goto write_ignore_32;
    case VRANGE32(GICD_CPENDSGIR, GICD_CPENDSGIRN):
        if (dabt.size != DABT_WORD)
            goto bad_width;
        return 0;
    case VRANGE32(GICD_SPENDSGIR, GICD_SPENDSGIRN):
        if (dabt.size != DABT_WORD)
            goto bad_width;
        return 0;
    case VRANGE32(0x0F30, 0x60FC):
        goto write_reserved;
    case VRANGE64(GICD_IROUTER32, GICD_IROUTER1019):
    {
        u64 irouter;

        if (!vgic_reg64_check_access(dabt))
            goto bad_width;

        rank = vgic_rank_offset(v, 64, gicd_reg - GICD_IROUTER,
                                DABT_DOUBLE_WORD);
        if (rank == NULL)
            goto write_ignore;

        vgic_lock_rank(v, rank, flags);
        irouter = vgic_fetch_irouter(rank, gicd_reg - GICD_IROUTER);
        vreg_reg64_update(&irouter, r, info);
        vgic_store_irouter(v->hypos, rank, gicd_reg - GICD_IROUTER, irouter);
        vgic_unlock_rank(v, rank, flags);
        return 1;
    }
    case VRANGE32(0x7FE0, 0xBFFC):
        goto write_reserved;
    case VRANGE32(0xC000, 0xFFCC):
        goto write_impl_defined;
    case VRANGE32(0xFFD0, 0xFFE4):
        goto write_impl_defined;
    case VREG32(GICD_PIDR2):
        goto write_ignore_32;
    case VRANGE32(0xFFEC, 0xFFFC):
        goto write_impl_defined;

    default:
        MSGH("%pv: vGICD: unhandled write r%d=%016lx offset %#08x\n",
             v, dabt.reg, r, gicd_reg);
        goto write_ignore;
    }

bad_width:
    MSGH("%pv: vGICD: bad write width %d r%d=%016lx offset %#08x\n",
         v, dabt.size, dabt.reg, r, gicd_reg);
    return 0;

write_ignore_32:
    if (dabt.size != DABT_WORD)
        goto bad_width;
    return 1;

write_ignore:
    return 1;

write_impl_defined:
    MSGH("%pv: vGICD: WI on implementation defined register offset %#08x\n",
         v, gicd_reg);
    return 1;

write_reserved:
    MSGH("%pv: vGICD: WI on reserved register offset %#08x\n",
         v, gicd_reg);
    return 1;
}

static bool vgic_v3_to_sgi(struct vcpu *v, u64 sgir)
{
    int virq;
    int irqmode;
    enum gic_sgi_mode sgi_mode;
    struct sgi_target target;

    sgi_target_init(&target);
    irqmode = (sgir >> ICH_SGI_IRQMODE_SHIFT)
              & ICH_SGI_IRQMODE_MASK;
    virq = (sgir >> ICH_SGI_IRQ_SHIFT ) & ICH_SGI_IRQ_MASK;

    switch (irqmode) {
    case ICH_SGI_TARGET_LIST:
        target.aff1 = (sgir >> ICH_SGI_AFFINITY_LEVEL(1))
                      & ICH_SGI_AFFx_MASK;
        target.list = sgir & ICH_SGI_TARGETLIST_MASK;
        sgi_mode = SGI_TARGET_LIST;
        break;
    case ICH_SGI_TARGET_OTHERS:
        sgi_mode = SGI_TARGET_OTHERS;
        break;
    default:
        MSGH("Wrong irq mode in SGI1R_EL1 register\n");
        return false;
    }

    return vgic_to_sgi(v, sgir, sgi_mode, virq, &target);
}

static bool vgic_v3_emulate_sgi1r(struct cpu_user_regs *regs, u64 *r,
                                  bool read)
{
    /* WO */
    if (!read)
        return vgic_v3_to_sgi(current, *r);
    else {
        MSGH("Reading SGI1R_EL1 - WO register\n");
        return false;
    }
}

static bool vgic_v3_emulate_sysreg(struct cpu_user_regs *regs, union hsr hsr)
{
    struct hsr_sysreg sysreg = hsr.sysreg;

    ASSERT (hsr.ec == HSR_EC_SYSREG);

    switch (hsr.bits & HSR_SYSREG_REGS_MASK) {
    case HSR_SYSREG_ICC_SGI1R_EL1:
        return vreg_emulate_sysreg(regs, hsr, vgic_v3_emulate_sgi1r);

    default:
        return false;
    }
}

static bool vgic_v3_emulate_cp64(struct cpu_user_regs *regs, union hsr hsr)
{

    switch (hsr.bits & HSR_CP64_REGS_MASK) {
    case HSR_CPREG64(ICC_SGI1R):
        return vreg_emulate_cp64(regs, hsr, vgic_v3_emulate_sgi1r);
    default:
        return false;
    }
}

static bool vgic_v3_emulate_reg(struct cpu_user_regs *regs, union hsr hsr)
{
    switch (hsr.ec) {
    case HSR_EC_SYSREG:
        return vgic_v3_emulate_sysreg(regs, hsr);
    case HSR_EC_CP15_64:
        return vgic_v3_emulate_cp64(regs, hsr);
    default:
        return false;
    }
}

static const struct mmio_handler_ops vgic_rdistr_mmio_handler = {
    .read  = vgic_v3_rdistr_mmio_read,
    .write = vgic_v3_rdistr_mmio_write,
};

static const struct mmio_handler_ops vgic_distr_mmio_handler = {
    .read  = vgic_v3_distr_mmio_read,
    .write = vgic_v3_distr_mmio_write,
};

static int vgic_v3_vcpu_init(struct vcpu *v)
{
    int i;
    hpa_t rdist_base;
    struct vgic_rdist_region *region;
    unsigned int last_cpu;

    struct hypos *d = v->hypos;

    for (i = 1; i < d->arch.vgic.nr_regions; i++) {
        if (v->vcpu_id < d->arch.vgic.rdist_regions[i].first_cpu)
            break;
    }

    region = &d->arch.vgic.rdist_regions[i - 1];

    rdist_base = region->base;
    rdist_base += (v->vcpu_id - region->first_cpu) * GICV3_GICR_SIZE;

    if ((rdist_base < region->base) ||
        ((rdist_base + GICV3_GICR_SIZE)
         > (region->base + region->size))) {
        MSGH("d%u: Unable to find a re-distributor for VCPU %u\n",
             d->hypos_id, v->vcpu_id);
        return -EINVAL;
    }

    v->arch.vgic.rdist_base = rdist_base;
    last_cpu = (region->size / GICV3_GICR_SIZE) + region->first_cpu - 1;

    if ( v->vcpu_id == last_cpu || (v->vcpu_id == (d->max_vcpus - 1)) )
        v->arch.vgic.flags |= VGIC_V3_RDIST_LAST;

    return 0;
}

static inline unsigned int vgic_v3_max_rdist_count(struct hypos *d)
{
    return hypos_use_host_layout(d) ? vgic_v3_hw.nr_rdist_regions :
                                       GUEST_GICV3_RDIST_REGIONS;
}

static int vgic_v3_hypos_init(struct hypos *d)
{
    struct vgic_rdist_region *rdist_regions;
    int rdist_count, i, ret;

    rdist_count = vgic_v3_max_rdist_count(d);

    rdist_regions = xzalloc_array(struct vgic_rdist_region, rdist_count);
    if (!rdist_regions)
        return -ENOMEM;

    d->arch.vgic.nr_regions = rdist_count;
    d->arch.vgic.rdist_regions = rdist_regions;

    rwlock_init(&d->arch.vgic.pend_lpi_tree_lock);
    radix_tree_init(&d->arch.vgic.pend_lpi_tree);

    if (hypos_use_host_layout(d)) {
        unsigned int first_cpu = 0;

        d->arch.vgic.dbase = vgic_v3_hw.dbase;

        for (i = 0; i < vgic_v3_hw.nr_rdist_regions; i++) {
            hpa_t size = vgic_v3_hw.regions[i].size;

            d->arch.vgic.rdist_regions[i].base = vgic_v3_hw.regions[i].base;
            d->arch.vgic.rdist_regions[i].size = size;

            /* Set the first CPU handled by this region */
            d->arch.vgic.rdist_regions[i].first_cpu = first_cpu;

            first_cpu += size / GICV3_GICR_SIZE;

            if ( first_cpu >= d->max_vcpus )
                break;
        }

        d->arch.vgic.nr_regions = i + 1;

        d->arch.vgic.intid_bits = vgic_v3_hw.intid_bits;
    } else {
        d->arch.vgic.dbase = GUEST_GICV3_GICD_BASE;

        BUILD_BUG_ON(GUEST_GICV3_RDIST_REGIONS != 1);

        BUILD_BUG_ON((GUEST_GICV3_GICR0_SIZE / GICV3_GICR_SIZE) < MAX_VIRT_CPUS);
        d->arch.vgic.rdist_regions[0].base = GUEST_GICV3_GICR0_BASE;
        d->arch.vgic.rdist_regions[0].size = GUEST_GICV3_GICR0_SIZE;
        d->arch.vgic.rdist_regions[0].first_cpu = 0;
        d->arch.vgic.intid_bits = 10;
    }

    ret = vgic_v3_its_init_hypos(d);
    if (ret)
        return ret;

    register_mmio_handler(d, &vgic_distr_mmio_handler, d->arch.vgic.dbase,
                          KB(64), NULL);

    for (i = 0; i < d->arch.vgic.nr_regions; i++) {
        struct vgic_rdist_region *region = &d->arch.vgic.rdist_regions[i];

        register_mmio_handler(d, &vgic_rdistr_mmio_handler,
                              region->base, region->size, region);
    }

    d->arch.vgic.ctlr = VGICD_CTLR_DEFAULT;

    return 0;
}

static void vgic_v3_hypos_free(struct hypos *d)
{
    vgic_v3_its_free_hypos(d);

    radix_tree_destroy(&d->arch.vgic.pend_lpi_tree, NULL);
    free(d->arch.vgic.rdist_regions);
}

static struct pending_irq *vgic_v3_lpi_to_pending(struct hypos *d,
                                                  unsigned int lpi)
{
    struct pending_irq *pirq;

    read_lock(&d->arch.vgic.pend_lpi_tree_lock);
    pirq = radix_tree_lookup(&d->arch.vgic.pend_lpi_tree, lpi);
    read_unlock(&d->arch.vgic.pend_lpi_tree_lock);

    return pirq;
}

static int vgic_v3_lpi_get_priority(struct hypos *d, u32 vlpi)
{
    struct pending_irq *p = vgic_v3_lpi_to_pending(d, vlpi);

    ASSERT(p);

    return p->lpi_priority;
}

static const struct vgic_ops v3_ops = {
    .vcpu_init   = vgic_v3_vcpu_init,
    .hypos_init = vgic_v3_hypos_init,
    .hypos_free = vgic_v3_hypos_free,
    .emulate_reg  = vgic_v3_emulate_reg,
    .lpi_to_pending = vgic_v3_lpi_to_pending,
    .lpi_get_priority = vgic_v3_lpi_get_priority,
};

int vgic_v3_init(struct hypos *d, unsigned int *mmio_count)
{
    if (!vgic_v3_hw.enabled) {
        MSGH("d%d: vGICv3 is not supported on this platform.\n",
             d->hypos_id);
        return -ENODEV;
    }

    *mmio_count = vgic_v3_max_rdist_count(d) + 1;

    *mmio_count += vgic_v3_its_count(d);

    register_vgic_ops(d, &v3_ops);

    return 0;
}

// --------------------------------------------------------------

static inline struct vgic_irq_rank *vgic_get_rank(struct vcpu *v,
                                                  unsigned int rank)
{
    if (rank == 0)
        return v->arch.vgic.private_irqs;
    else if (rank <= DOMAIN_NR_RANKS(v->hypos))
        return &v->hypos->arch.vgic.shared_irqs[rank - 1];
    else
        return NULL;
}

struct vgic_irq_rank *vgic_rank_offset(struct vcpu *v, unsigned int b,
                                       unsigned int n, unsigned int s)
{
    unsigned int rank = REG_RANK_NR(b, (n >> s));

    return vgic_get_rank(v, rank);
}

struct vgic_irq_rank *vgic_rank_irq(struct vcpu *v, unsigned int irq)
{
    unsigned int rank = irq / 32;

    return vgic_get_rank(v, rank);
}

void vgic_init_pending_irq(struct pending_irq *p, unsigned int virq)
{
    BUILD_BUG_ON(BIT(sizeof(p->lpi_vcpu_id) * 8, UL) < MAX_VIRT_CPUS);

    memset(p, 0, sizeof(*p));
    INIT_LIST_HEAD(&p->inflight);
    INIT_LIST_HEAD(&p->lr_queue);
    p->irq = virq;
    p->lpi_vcpu_id = INVALID_VCPU_ID;
}

static void vgic_rank_init(struct vgic_irq_rank *rank, uint8_t index,
                           unsigned int vcpu)
{
    unsigned int i;

    BUILD_BUG_ON((1 << (sizeof(rank->vcpu[0]) * 8)) < MAX_VIRT_CPUS);

    spin_lock_init(&rank->lock);

    rank->index = index;

    for (i = 0; i < NR_INTERRUPT_PER_RANK; i++)
        write_atomic(&rank->vcpu[i], vcpu);
}

int hypos_vgic_register(struct hypos *d, unsigned int *mmio_count)
{
    switch (d->arch.vgic.version) {
    case GIC_V3:
        if (vgic_v3_init(d, mmio_count))
           return -ENODEV;
        break;
    default:
        MSGH("d%d: Unknown vGIC version %u\n",
             d->hypos_id, d->arch.vgic.version);
        return -ENODEV;
    }

    return 0;
}

int hypos_vgic_init(struct hypos *d, unsigned int nr_spis)
{
    int i;
    int ret;

    d->arch.vgic.ctlr = 0;

    nr_spis = ROUNDUP(nr_spis, 32);

    if (nr_spis > (1020 - NR_LOCAL_IRQS))
        return -EINVAL;

    d->arch.vgic.nr_spis = nr_spis;

    spin_lock_init(&d->arch.vgic.lock);

    d->arch.vgic.shared_irqs =
        xzalloc_array(struct vgic_irq_rank, DOMAIN_NR_RANKS(d));
    if (d->arch.vgic.shared_irqs == NULL)
        return -ENOMEM;

    d->arch.vgic.pending_irqs =
        xzalloc_array(struct pending_irq, d->arch.vgic.nr_spis);
    if (d->arch.vgic.pending_irqs == NULL)
        return -ENOMEM;

    for (i=0; i<d->arch.vgic.nr_spis; i++)
        vgic_init_pending_irq(&d->arch.vgic.pending_irqs[i], i + 32);

    for (i = 0; i < DOMAIN_NR_RANKS(d); i++)
        vgic_rank_init(&d->arch.vgic.shared_irqs[i], i + 1, 0);

    ret = d->arch.vgic.handler->hypos_init(d);
    if (ret)
        return ret;

    d->arch.vgic.allocated_irqs =
        xzalloc_array(unsigned long, BITS_TO_LONGS(vgic_num_irqs(d)));
    if (!d->arch.vgic.allocated_irqs)
        return -ENOMEM;

    for (i = 0; i < NR_GIC_SGI; i++)
        set_bit(i, d->arch.vgic.allocated_irqs);

    return 0;
}

void register_vgic_ops(struct hypos *d, const struct vgic_ops *ops)
{
   d->arch.vgic.handler = ops;
}

void hypos_vgic_free(struct hypos *d)
{
    int i;
    int ret;

    for (i = 0; i < (d->arch.vgic.nr_spis); i++) {
        struct pending_irq *p = spi_to_pending(d, i + 32);

        if (p->desc) {
            ret = release_guest_irq(d, p->irq);
            if (ret)
                MSGH("d%u: Failed to release virq %u ret = %d\n",
                        d->hypos_id, p->irq, ret);
        }
    }

    if (d->arch.vgic.handler)
        d->arch.vgic.handler->hypos_free(d);
    free(d->arch.vgic.shared_irqs);
    free(d->arch.vgic.pending_irqs);
    free(d->arch.vgic.allocated_irqs);
}

int vcpu_vgic_init(struct vcpu *v)
{
    int i;

    v->arch.vgic.private_irqs = xzalloc(struct vgic_irq_rank);
    if (v->arch.vgic.private_irqs == NULL)
      return -ENOMEM;

    /* SGIs/PPIs are always routed to this VCPU */
    vgic_rank_init(v->arch.vgic.private_irqs, 0, v->vcpu_id);

    v->hypos->arch.vgic.handler->vcpu_init(v);

    memset(&v->arch.vgic.pending_irqs, 0, sizeof(v->arch.vgic.pending_irqs));
    for (i = 0; i < 32; i++)
        vgic_init_pending_irq(&v->arch.vgic.pending_irqs[i], i);

    INIT_LIST_HEAD(&v->arch.vgic.inflight_irqs);
    INIT_LIST_HEAD(&v->arch.vgic.lr_pending);
    spin_lock_init(&v->arch.vgic.lock);

    return 0;
}

int vcpu_vgic_free(struct vcpu *v)
{
    free(v->arch.vgic.private_irqs);
    return 0;
}

struct vcpu *vgic_get_target_vcpu(struct vcpu *v, unsigned int virq)
{
    struct vgic_irq_rank *rank = vgic_rank_irq(v, virq);
    int target = read_atomic(&rank->vcpu[virq & INTERRUPT_RANK_MASK]);
    return v->hypos->vcpu[target];
}

static int vgic_get_virq_priority(struct vcpu *v, unsigned int virq)
{
    struct vgic_irq_rank *rank;

    if (is_lpi(virq))
        return v->hypos->arch.vgic.handler->lpi_get_priority(v->hypos, virq);

    rank = vgic_rank_irq(v, virq);
    return ACCESS_ONCE(rank->priority[virq & INTERRUPT_RANK_MASK]);
}

bool vgic_migrate_irq(struct vcpu *old, struct vcpu *new, unsigned int irq)
{
    unsigned long flags;
    struct pending_irq *p;

    ASSERT(!is_lpi(irq));

    spin_lock_irqsave(&old->arch.vgic.lock, flags);

    p = irq_to_pending(old, irq);

    if (p->desc == NULL) {
        spin_unlock_irqrestore(&old->arch.vgic.lock, flags);
        return true;
    }

    if (test_bit(GIC_IRQ_GUEST_MIGRATING, &p->status)) {
        MSGH("irq %u migration failed: requested while in progress\n", irq);
        spin_unlock_irqrestore(&old->arch.vgic.lock, flags);
        return false;
    }

    if (list_empty(&p->inflight)) {
        irq_set_affinity(p->desc, cpumask_of(new->processor));
        spin_unlock_irqrestore(&old->arch.vgic.lock, flags);
        return true;
    }

    if (!list_empty(&p->lr_queue)) {
        vgic_remove_irq_from_queues(old, p);
        irq_set_affinity(p->desc, cpumask_of(new->processor));
        spin_unlock_irqrestore(&old->arch.vgic.lock, flags);
        vgic_inject_irq(new->hypos, new, irq, true);
        return true;
    }

    if (!list_empty(&p->inflight))
        set_bit(GIC_IRQ_GUEST_MIGRATING, &p->status);

    spin_unlock_irqrestore(&old->arch.vgic.lock, flags);
    return true;
}

void arch_move_irqs(struct vcpu *v)
{
    const cpumask_t *cpu_mask = cpumask_of(v->processor);
    struct hypos *d = v->hypos;
    struct pending_irq *p;
    struct vcpu *v_target;
    int i;

    ASSERT(!is_lpi(vgic_num_irqs(d) - 1));

    for (i = 32; i < vgic_num_irqs(d); i++) {
        v_target = vgic_get_target_vcpu(v, i);
        p = irq_to_pending(v_target, i);

        if (v_target == v && !test_bit(GIC_IRQ_GUEST_MIGRATING, &p->status))
            irq_set_affinity(p->desc, cpu_mask);
    }
}

void vgic_disable_irqs(struct vcpu *v, u32 r, unsigned int n)
{
    const unsigned long mask = r;
    struct pending_irq *p;
    struct irq_desc *desc;
    unsigned int irq;
    unsigned long flags;
    unsigned int i = 0;
    struct vcpu *v_target;

    ASSERT(!is_lpi(32 * n + 31));

    while ((i = find_next_bit(&mask, 32, i)) < 32) {
        irq = i + (32 * n);
        v_target = vgic_get_target_vcpu(v, irq);

        spin_lock_irqsave(&v_target->arch.vgic.lock, flags);
        p = irq_to_pending(v_target, irq);
        clear_bit(GIC_IRQ_GUEST_ENABLED, &p->status);
        gic_remove_from_lr_pending(v_target, p);
        desc = p->desc;
        spin_unlock_irqrestore(&v_target->arch.vgic.lock, flags);

        if (desc != NULL) {
            spin_lock_irqsave(&desc->lock, flags);
            desc->handler->disable(desc);
            spin_unlock_irqrestore(&desc->lock, flags);
        }

        i++;
    }
}

#define VGIC_ICFG_MASK(intr) (1U << ((2 * ((intr) % 16)) + 1))

static inline unsigned int vgic_get_virq_type(struct vcpu *v,
                                              unsigned int n,
                                              unsigned int index)
{
    struct vgic_irq_rank *r = vgic_get_rank(v, n);
    u32 tr = r->icfg[index >> 4];

    ASSERT(spin_is_locked(&r->lock));

    if (tr & VGIC_ICFG_MASK(index))
        return IRQ_TYPE_EDGE_RISING;
    else
        return IRQ_TYPE_LEVEL_HIGH;
}

void vgic_enable_irqs(struct vcpu *v, u32 r, unsigned int n)
{
    const unsigned long mask = r;
    struct pending_irq *p;
    unsigned int irq;
    unsigned long flags;
    unsigned int i = 0;
    struct vcpu *v_target;
    struct hypos *d = v->hypos;

    ASSERT(!is_lpi(32 * n + 31));

    while ((i = find_next_bit(&mask, 32, i)) < 32) {
        irq = i + (32 * n);
        v_target = vgic_get_target_vcpu(v, irq);
        spin_lock_irqsave(&v_target->arch.vgic.lock, flags);
        p = irq_to_pending(v_target, irq);
        set_bit(GIC_IRQ_GUEST_ENABLED, &p->status);

        if (!list_empty(&p->inflight) && !test_bit(GIC_IRQ_GUEST_VISIBLE,
                    &p->status))
            gic_raise_guest_irq(v_target, irq, p->priority);
        spin_unlock_irqrestore(&v_target->arch.vgic.lock, flags);

        if (p->desc != NULL) {
            irq_set_affinity(p->desc, cpumask_of(v_target->processor));
            spin_lock_irqsave(&p->desc->lock, flags);

            ASSERT(irq >= 32);
            if (irq_type_set_by_hypos(d))
                gic_set_irq_type(p->desc, vgic_get_virq_type(v, n, i));
            p->desc->handler->enable(p->desc);
            spin_unlock_irqrestore(&p->desc->lock, flags);
        }

        i++;
    }
}

void vgic_set_irqs_pending(struct vcpu *v, u32 r, unsigned int rank)
{
    const unsigned long mask = r;
    unsigned int i;
    bool private = rank == 0;

    ASSERT(!is_lpi(32 * rank + 31));

    for_each_set_bit(i, &mask, 32) {
        unsigned int irq = i + 32 * rank;

        if (!private) {
            struct pending_irq *p = spi_to_pending(v->hypos, irq);

            if (p->desc != NULL) {
                unsigned long flags;

                spin_lock_irqsave(&p->desc->lock, flags);
                gic_set_pending_state(p->desc, true);
                spin_unlock_irqrestore(&p->desc->lock, flags);
                continue;
            }
        }

        vgic_inject_irq(v->hypos, private ? v : NULL, irq, true);
    }
}

bool vgic_to_sgi(struct vcpu *v, register_t sgir, enum gic_sgi_mode irqmode,
                 int virq, const struct sgi_target *target)
{
    struct hypos *d = v->hypos;
    int vcpuid;
    int i;
    unsigned int base;
    unsigned long int bitmap;

    ASSERT(virq < 16);

    switch (irqmode) {
    case SGI_TARGET_LIST:
        base = target->aff1 << 4;
        bitmap = target->list;
        for_each_set_bit(i, &bitmap, sizeof(target->list) * 8) {
            vcpuid = base + i;
            if (vcpuid >= d->max_vcpus || d->vcpu[vcpuid] == NULL ||
                !is_vcpu_online(d->vcpu[vcpuid])) {
                MSGH("VGIC: write r=%016lx \
                     target->list=%hx, wrong CPUTargetList \n",
                     sgir, target->list);
                continue;
            }
            vgic_inject_irq(d, d->vcpu[vcpuid], virq, true);
        }
        break;
    case SGI_TARGET_OTHERS:
        for (i = 0; i < d->max_vcpus; i++) {
            if (i != current->vcpu_id && d->vcpu[i] != NULL &&
                 is_vcpu_online(d->vcpu[i]))
                vgic_inject_irq(d, d->vcpu[i], virq, true);
        }
        break;
    case SGI_TARGET_SELF:
        vgic_inject_irq(d, current, virq, true);
        break;
    default:
        MSGH("vGICD:unhandled GICD_SGIR write %016lx \
             with wrong mode\n", sgir);
        return false;
    }

    return true;
}

struct pending_irq *irq_to_pending(struct vcpu *v, unsigned int irq)
{
    struct pending_irq *n;

    if (irq < 32)
        n = &v->arch.vgic.pending_irqs[irq];
    else if (is_lpi(irq))
        n = v->hypos->arch.vgic.handler->lpi_to_pending(v->hypos, irq);
    else
        n = &v->hypos->arch.vgic.pending_irqs[irq - 32];
    return n;
}

struct pending_irq *spi_to_pending(struct hypos *d, unsigned int irq)
{
    ASSERT(irq >= NR_LOCAL_IRQS);

    return &d->arch.vgic.pending_irqs[irq - 32];
}

void vgic_clear_pending_irqs(struct vcpu *v)
{
    struct pending_irq *p, *t;
    unsigned long flags;

    spin_lock_irqsave(&v->arch.vgic.lock, flags);
    list_for_each_entry_safe(p, t, &v->arch.vgic.inflight_irqs,
            inflight);
    list_del_init(&p->inflight);
    gic_clear_pending_irqs(v);
    spin_unlock_irqrestore(&v->arch.vgic.lock, flags);
}

void vgic_remove_irq_from_queues(struct vcpu *v,
                                 struct pending_irq *p)
{
    ASSERT(spin_is_locked(&v->arch.vgic.lock));

    clear_bit(GIC_IRQ_GUEST_QUEUED, &p->status);
    list_del_init(&p->inflight);
    gic_remove_from_lr_pending(v, p);
}

void vgic_inject_irq(struct hypos *d, struct vcpu *v,
                     unsigned int virq,
                     bool level)
{
    uint8_t priority;
    struct pending_irq *iter, *n;
    unsigned long flags;

    if (!level)
        return;

    if (!v) {
        ASSERT(virq >= 32 && virq <= vgic_num_irqs(d));

        v = vgic_get_target_vcpu(d->vcpu[0], virq);
    };

    spin_lock_irqsave(&v->arch.vgic.lock, flags);

    n = irq_to_pending(v, virq);

    if (unlikely(!n)) {
        spin_unlock_irqrestore(&v->arch.vgic.lock, flags);
        return;
    }

    /* vcpu offline */
    if (test_bit(_VPF_down, &v->pause_flags)) {
        spin_unlock_irqrestore(&v->arch.vgic.lock, flags);
        return;
    }

    set_bit(GIC_IRQ_GUEST_QUEUED, &n->status);

    if (!list_empty(&n->inflight)) {
        gic_raise_inflight_irq(v, virq);
        goto out;
    }

    priority = vgic_get_virq_priority(v, virq);
    n->priority = priority;

    if (test_bit(GIC_IRQ_GUEST_ENABLED, &n->status))
        gic_raise_guest_irq(v, virq, priority);

    list_for_each_entry(iter, &v->arch.vgic.inflight_irqs,
            inflight) {
        if (iter->priority > priority) {
            list_add_tail(&n->inflight, &iter->inflight);
            goto out;
        }
    }
    list_add_tail(&n->inflight, &v->arch.vgic.inflight_irqs);
out:
    spin_unlock_irqrestore(&v->arch.vgic.lock, flags);

    vcpu_kick(v);

    return;
}

bool vgic_evtchn_irq_pending(struct vcpu *v)
{
    struct pending_irq *p;

    p = irq_to_pending(v, v->hypos->arch.evtchn_irq);
    ASSERT(!is_lpi(v->hypos->arch.evtchn_irq));

    return list_empty(&p->inflight);
}

bool vgic_emulate(struct cpu_user_regs *regs, union hsr hsr)
{
    struct vcpu *v = current;

    ASSERT(v->hypos->arch.vgic.handler->emulate_reg != NULL);

    return v->hypos->arch.vgic.handler->emulate_reg(regs, hsr);
}

bool vgic_reserve_virq(struct hypos *d, unsigned int virq)
{
    if (virq >= vgic_num_irqs(d))
        return false;

    return !test_and_set_bit(virq, d->arch.vgic.allocated_irqs);
}

int vgic_allocate_virq(struct hypos *d, bool spi)
{
    int first, end;
    unsigned int virq;

    if (!spi) {
        first = 16;
        end = 32;
    } else {
        first = 32;
        end = vgic_num_irqs(d);
    }

    do {
        virq = find_next_zero_bit(d->arch.vgic.allocated_irqs, end, first);
        if (virq >= end)
            return -1;
    } while (test_and_set_bit(virq, d->arch.vgic.allocated_irqs));

    return virq;
}

void vgic_free_virq(struct hypos *d, unsigned int virq)
{
    clear_bit(virq, d->arch.vgic.allocated_irqs);
}

unsigned int vgic_max_vcpus(unsigned int domctl_vgic_version)
{
    switch (hypos_vgic_version) {
    case HYPOS_CFG_GIC_V2:
        return 8;
    case HYPOS_CFG_GIC_V3:
        return 4096;
    default:
        return 0;
    }
}

void vgic_check_inflight_irqs_pending(struct hypos *d, struct vcpu *v,
                                      unsigned int rank, u32 r)
{
    const unsigned long mask = r;
    unsigned int i;

    for_each_set_bit(i, &mask, 32) {
        struct pending_irq *p;
        struct vcpu *v_target;
        unsigned long flags;
        unsigned int irq = i + 32 * rank;

        v_target = vgic_get_target_vcpu(v, irq);

        spin_lock_irqsave(&v_target->arch.vgic.lock, flags);

        p = irq_to_pending(v_target, irq);

        if (p && !list_empty(&p->inflight))
            MSGH("%pv trying to clear pending interrupt %u.\n",
                 v, irq);

        spin_unlock_irqrestore(&v_target->arch.vgic.lock, flags);
    }
}

// --------------------------------------------------------------
#else

void vgic_v3_setup_hw(hpa_t dbase,
                      unsigned int nr_rdist_regions,
                      const struct rdist_region *regions,
                      unsigned int intid_bits)
{

}

struct vgic_irq_rank *vgic_rank_irq(struct vcpu *v,
                                    unsigned int irq)
{
    return NULL;
}

struct pending_irq *irq_to_pending(struct vcpu *v,
                                   unsigned int irq)
{
    return NULL;
}

struct vcpu *vgic_get_target_vcpu(struct vcpu *v,
                                  unsigned int virq)
{
    return NULL;
}

void vgic_inject_irq(struct hypos *d, struct vcpu *v,
                     unsigned int virq,
                     bool level)
{
    /* XXX: Not Implemented */
}

// --------------------------------------------------------------
#endif
