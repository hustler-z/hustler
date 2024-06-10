/**
 * Hustler's Project
 *
 * File:  gicv3.c
 * Date:  2024/05/22
 * Usage: gic3 initialization
 */

#include <generic/gicv3.h>
#include <asm/barrier.h>
#include <lib/strops.h>

struct gicv3_its_ctlr_if *gic_its;
struct gicv3_its_int_if  *gic_its_ints;
struct gicv3_dist_if     *gic_dist;
struct gicv3_rdist_if    *gic_rdist;

static u32 gic_max_rd;
static u32 gic_addr_valid;

// --------------------------------------------------------------

/* Basic GICv3 Interfaces
 */

/*
 * Sets the address of the Distributor and Redistributors
 * dist   => virtual address of the Distributor
 * rdist => virtual address of the first Redistributor
 */
void set_gic_base(void *dist, void *rdist)
{
    u32 index = 0;

    gic_dist  = (struct gicv3_dist_if *)dist;
    gic_rdist = (struct gicv3_rdist_if *)rdist;
    gic_addr_valid = 1;
    while (!(gic_rdist[index].lpis.gicr_typer[0] & (1 << 4)))
        index++;

    gic_max_rd = index;
}

u32 get_ext_ppi(u32 rd)
{
    return (((gic_rdist[rd].lpis.gicr_typer[0] >> 27) & 0x1F) + 1) * 32;
}

u32 get_spi(void)
{
    return ((gic_dist->gicd_typer & 0x1F) + 1)* 32;
}

u32 get_ext_spi(void)
{
    if (((gic_dist->gicd_typer >> 8) & 0x1) == 1)
        return (((gic_dist->gicd_typer >> 27) & 0x1F) + 1) * 32;
    else
        return 0;
}

static u32 is_valid_ext_ppi(u32 rd, u32 id)
{
    u32 max_ppi;

    max_ppi = ((gic_rdist[rd].lpis.gicr_typer[0] >> 27) & 0x1F);
    if (max_ppi == 0)
        return 0;
    else if ((max_ppi == 1) && (id > 1087))
        return 0;

    return 1;
}

static u32 is_valid_ext_spi(u32 id)
{
    u32 max_spi;

    if (((gic_dist->gicd_typer >> 8) & 0x1) == 0)
        return 0;
    else {
        max_spi = ((gic_dist->gicd_typer >> 27) & 0x1F);
        max_spi = (max_spi + 1) * 32;
        max_spi = max_spi + 4096;

        if (!(id < max_spi))
            return 0;
    }

    return 1;
}

u32 enable_gic(void)
{
    if (!gic_addr_valid)
        return 1;

    gic_dist->gicd_ctlr = (1 << 5) | (1 << 4);
    gic_dist->gicd_ctlr = 7 | (1 << 5) | (1 << 4);

    return 0;
}

u32 get_rdist_id(u32 affinity)
{
    u32 index = 0;

    if (!gic_addr_valid)
        return 0xFFFFFFFF;

    do {
        if (gic_rdist[index].lpis.gicr_typer[1] == affinity)
            return index;

        index++;
    } while(index <= gic_max_rd);

    return 0xFFFFFFFF;
}

u32 wakeup_rdist(u32 rd)
{
    u32 tmp;

    if (!gic_addr_valid)
        return 1;

    tmp = gic_rdist[rd].lpis.gicr_waker;
    tmp = tmp & ~0x2;
    gic_rdist[rd].lpis.gicr_waker = tmp;

    do {
        tmp = gic_rdist[rd].lpis.gicr_waker;
    } while((tmp & 0x4) != 0);

    return 0;
}

u32 enable_interrupt(u32 id, u32 rd)
{
    u32 bank, max_ppi, max_spi;
    u8* config;

    if (!gic_addr_valid)
        return 1;

    if (id < 31) {
        if (rd > gic_max_rd)
            return 1;

        id   = id & 0x1f;
        id   = 1 << id;
        gic_rdist[rd].sgis.gicr_isenabler[0] = id;
    } else if (id < 1020) {
        bank = id / 32;
        id   = id & 0x1f;
        id   = 1 << id;
        gic_dist->gicd_isenabler[bank] = id;
    } else if ((id > 1055) && (id < 1120)) {
        if (rd > gic_max_rd)
            return 1;
        if (!is_valid_ext_ppi(rd, id))
            return 1;

        id   = id - 1024;
        bank = id / 32;
        id   = id & 0x1F;
        id   = 1 << id;
        gic_rdist[rd].sgis.gicr_isenabler[bank] = id;
    } else if ((id > 4095) && (id < 5120)) {
        if (!is_valid_ext_spi(id))
            return 1;

        id   = id - 4096;
        bank = id / 32;
        id   = id & 0x1F;
        id   = 1 << id;
        gic_dist->gicd_isenablere[bank] = id;
    } else
        return 1;

    return 0;
}

u32 disable_interrupt(u32 id, u32 rd)
{
    u32 bank, max_ppi, max_spi;
    u8* config;

    if (!gic_addr_valid)
        return 1;

    if (id < 31) {
        if (rd > gic_max_rd)
            return 1;

        id = id & 0x1F;
        id = 1 << id;

        gic_rdist[rd].sgis.gicr_icenabler[0] = id;
    } else if (id < 1020) {

        bank = id / 32;
        id = id & 0x1F;
        id = 1 << id;

        gic_dist->gicd_icenabler[bank] = id;
    } else if ((id > 1055) && (id < 1120)) {
        if (rd > gic_max_rd)
            return 1;

        if (!is_valid_ext_ppi(rd, id))
            return 1;

        id = id - 1024;
        bank = id / 32;
        id = id & 0x1F;
        id = 1 << id;

        gic_rdist[rd].sgis.gicr_icenabler[bank] = id;
    } else if ((id > 4095) && (id < 5120)) {
        if (!is_valid_ext_spi(id))
            return 1;

        id = id - 4096;
        bank = id / 32;
        id = id & 0x1F;
        id = 1 << id;
        gic_dist->gicd_icenablere[bank] = id;
    } else
        return 1;

    return 0;
}

u32 set_interrupt_priority(u32 id, u32 rd, u8 priority)
{
    u8* config;
    u32 max_ppi, max_spi;

    if (!gic_addr_valid)
        return 1;

    if (id < 31) {
        if (rd > gic_max_rd)
            return 1;

        gic_rdist[rd].sgis.gicr_ipriorityr[id] = priority;
    } else if (id < 1020)
        gic_dist->gicd_ipriorityr[id] = priority;
    else if ((id > 1055) && (id < 1120)) {
        if (rd > gic_max_rd)
            return 1;

        if (!is_valid_ext_ppi(rd, id))
            return 1;

        id = id - 1024;
        gic_rdist[rd].sgis.gicr_ipriorityr[id] = priority;
    } else if ((id > 4095) && (id < 5120)) {

        if (!is_valid_ext_spi(id))
            return 1;

        gic_dist->gicd_ipriorityre[(id - 4096)] = priority;
    } else
        return 1;

    return 0;
}

u32 set_interrupt_type(u32 id, u32 rd, u32 type)
{
    u8* config;
    u32 bank, tmp, max_spi, conf = 0x0;

    if (!gic_addr_valid)
        return 1;

    if (id < 31)
        return 1;
    else if (id < 1020) {
        type = type & 0x3;
        bank = id / 16;
        id = id & 0x0F;
        id = id * 2;
        conf = conf << id;

        tmp = gic_dist->gicd_icfgr[bank];
        tmp = tmp & ~(0x3 << id);
        tmp = tmp | conf;
        gic_dist->gicd_icfgr[bank] = tmp;
    } else if ((id > 4095) && (id < 5120)) {
        if (!is_valid_ext_spi(id))
            return 1;

        type = type & 0x3;
        id = id - 4096;
        bank = id / 16;
        id = id & 0x0F;
        id = id * 2;
        conf = conf << id;

        tmp = gic_dist->gicd_icfgre[bank];
        tmp = tmp & ~(0x3 << id);
        tmp = tmp | conf;
        gic_dist->gicd_icfgre[bank] = tmp;
    } else
        return 1;

    return 0;
}

u32 set_interrupt_grp(u32 id, u32 rd, u32 security)
{
    u8* config;
    u32 bank, tmp, group, mod, max_ppi, max_spi;

    if (!gic_addr_valid)
        return 1;

    if (id < 31) {
        if (rd > gic_max_rd)
            return 1;

        id = id & 0x1F;
        id = 1 << id;

        group = gic_rdist[rd].sgis.gicr_igroupr[0];
        mod   = gic_rdist[rd].sgis.gicr_igrpmodr[0];

        switch (security) {
        case GICV3_GROUP0:
            group = (group & ~id);
            mod   = (mod   & ~id);
            break;
        case GICV3_GROUP1_SECURE:
            group = (group & ~id);
            mod   = (mod   | id);
            break;
        case GICV3_GROUP1_NON_SECURE:
            group = (group | id);
            mod   = (mod   & ~id);
            break;
        default:
            return 1;
        }

        gic_rdist[rd].sgis.gicr_igroupr[0] = group;
        gic_rdist[rd].sgis.gicr_igrpmodr[0] = mod;
    } else if (id < 1020) {
        bank = id / 32;
        id = id & 0x1F;
        id = 1 << id;
        group = gic_dist->gicd_igroupr[bank];
        mod   = gic_dist->gicd_grpmodr[bank];

        switch (security) {
        case GICV3_GROUP0:
            group = (group & ~id);
            mod   = (mod   & ~id);
            break;
        case GICV3_GROUP1_SECURE:
            group = (group & ~id);
            mod   = (mod   | id);
            break;
        case GICV3_GROUP1_NON_SECURE:
            group = (group | id);
            mod   = (mod   & ~id);
            break;
        default:
            return 1;
        }

        gic_dist->gicd_igroupr[bank] = group;
        gic_dist->gicd_grpmodr[bank] = mod;
    } else if ((id > 1055) && (id < 1120)) {
        if (rd > gic_max_rd)
            return 1;

        if (!is_valid_ext_ppi(rd, id))
            return 1;

        id = id - 1024;
        bank = id / 32;
        id = id & 0x1F;
        id = 1 << id;

        group = gic_rdist[rd].sgis.gicr_igroupr[bank];
        mod   = gic_rdist[rd].sgis.gicr_igrpmodr[bank];

        switch (security) {
        case GICV3_GROUP0:
            group = (group & ~id);
            mod   = (mod   & ~id);
            break;
        case GICV3_GROUP1_SECURE:
            group = (group & ~id);
            mod   = (mod   | id);
            break;
        case GICV3_GROUP1_NON_SECURE:
            group = (group | id);
            mod   = (mod   & ~id);
            break;
        default:
            return 1;
        }

        gic_rdist[rd].sgis.gicr_igroupr[bank] = group;
        gic_rdist[rd].sgis.gicr_igrpmodr[bank] = mod;
    } else if ((id > 4095) && (id < 5120)) {
        if (!is_valid_ext_spi(id))
            return 1;

        id = id - 4096;
        bank = id / 32;
        id = id & 0x1F;
        id = 1 << id;

        group = gic_dist->gicd_igroupre[bank];
        mod   = gic_dist->gicd_igrpmodre[bank];

        switch (security) {
        case GICV3_GROUP0:
            group = (group & ~id);
            mod   = (mod   & ~id);
            break;
        case GICV3_GROUP1_SECURE:
            group = (group & ~id);
            mod   = (mod   | id);
            break;
        case GICV3_GROUP1_NON_SECURE:
            group = (group | id);
            mod   = (mod   & ~id);
            break;
        default:
            return 1;
        }

        gic_dist->gicd_igroupre[bank] = group;
        gic_dist->gicd_igrpmodre[bank] = mod;
    } else
        return 1;

    return 0;
}

#define GICV3_ROUTE_AFF3_SHIFT           (8)

u32 set_interrupt_route(u32 id, u32 mode, u32 affinity)
{
    u64 tmp, max_spi;

    if (!gic_addr_valid)
        return 0xFFFFFFFF;

    if (!((id > 31) && (id < 1020))) {
        if (!((id > 4095) && (id < 5120)))
            return 1;

        if (!is_valid_ext_spi(id))
            return 1;
    }

    tmp = (u64)(affinity & 0x00FFFFFF) |
        (((u64)affinity & 0xFF000000) << GICV3_ROUTE_AFF3_SHIFT) |
        (u64)mode;
    if ((id > 31) && (id < 1020))
        gic_dist->gicd_router[id] = tmp;
    else
        gic_dist->gicd_routere[(id - 4096)] = tmp;

    return 0;
}

u32 set_interrupt_pending(u32 id, u32 rd)
{
    u8* config;
    u32 bank, tmp, max_ppi, max_spi;

    if (!gic_addr_valid)
        return 0xFFFFFFFF;

    if (id < 31) {
        if (rd > gic_max_rd)
            return 1;

        id = id & 0x1F;
        id = 1 << id;

        gic_rdist[rd].sgis.gicr_ispendr[0] = id;
    } else if (id < 1020) {
        bank = id / 32;
        id = id & 0x1F;

        id = 1 << id;
        gic_dist->gicd_ispendr[bank] = id;
    } else if ((id > 1055) && (id < 1120)) {
        if (rd > gic_max_rd)
            return 1;

        if (!is_valid_ext_ppi(rd, id))
            return 1;

        id = id - 1024;
        bank = id / 32;
        id = id & 0x1F;
        id = 1 << id;

        gic_rdist[rd].sgis.gicr_ispendr[bank] = id;
    } else if ((id > 4095) && (id < 5120)) {
        if (!is_valid_ext_spi(id))
            return 1;

        id = id - 4096;
        bank = id / 32;
        id = id & 0x1F;
        id = 1 << id;

        gic_dist->gicd_ispendre[bank] = id;
    } else
        return 1;

    return 0;
}

u32 clear_interrupt_pending(u32 id, u32 rd)
{
    u8* config;
    u32 bank, tmp, max_ppi, max_spi;

    if (!gic_addr_valid)
        return 0xFFFFFFFF;

    if (id < 31) {
        if (rd > gic_max_rd)
            return 1;

        id = id & 0x1F;
        id = 1 << id;
        gic_rdist[rd].sgis.gicr_icpendr[0] = id;

    } else if (id < 1020) {

        bank = id / 32;
        id = id & 0x1f;
        id = 1 << id;
        gic_dist->gicd_icpendr[bank] = id;
    } else if ((id > 1055) && (id < 1120)) {
        if (rd > gic_max_rd)
            return 1;

        if (!is_valid_ext_ppi(rd, id))
            return 1;

        id = id - 1024;
        bank = id / 32;
        id = id & 0x1F;
        id = 1 << id;
        gic_rdist[rd].sgis.gicr_icpendr[bank] = id;
    } else if ((id > 4095) && (id < 5120)) {
        if (!is_valid_ext_spi(id))
            return 1;

        id = id - 4096;
        bank = id / 32;
        id = id & 0x1F;
        id = 1 << id;
        gic_dist->gicd_icpendre[bank] = id;
    } else
        return 1;

    return 0;
}

// --------------------------------------------------------------

/* LPI Interfaces
 *
 * Locality-specific Peripheral Interrupts (LPIs) are edge-triggered
 * message-based interrupts that can use an Interrupt Translation
 * Service (ITS), if it is implemented, to route an interrupt to a
 * specific Redistributor and connected PE.
 */

void set_its_base(void* its_base)
{
    gic_its      = (struct gicv3_its_ctlr_if *)its_base;
    gic_its_ints = (struct gicv3_its_int_if *)(its_base + 0x010000);
}

u32 set_lpi_config_table_addr(u32 rd, u64 addr, u64 attributes,
        u32 idbits)
{
    u64 tmp = 0;


    if ((idbits < 14) || (idbits > 24))
        return 1;

    tmp = (1 << idbits) - 8192;
    idbits = idbits - 1;
    memset((void*)addr, 0, tmp);
    mb();

    addr =  addr & 0x0000FFFFFFFFF000;
    tmp  = addr | (attributes & 0x0700000000000F80);
    tmp  = tmp  | (idbits     & 0x000000000000001F);
    gic_rdist[rd].lpis.gicr_propbaser = tmp;

    return 0;
}

u32 set_lpi_pending_table_addr(u32 rd, u64 addr, u64 attributes,
        u32 idbits)
{
    u64 tmp = 0;

    if ((idbits < 14) || (idbits > 24))
        return 1;

    tmp = 1 << idbits;
    memset((void*)addr, 0, tmp);
    mb();

    addr = addr & 0x0000FFFFFFFF0000;
    tmp  = addr | (attributes & 0x0700000000000F80);
    gic_rdist[rd].lpis.gicr_pendbaser = tmp;

    return 0;
}

void enable_lpis(u32 rd)
{
    gic_rdist[rd].lpis.gicr_ctlr = (gic_rdist[rd].lpis.gicr_ctlr | 0x1);
    mb();
}

u32 get_rd_proc_nr(u32 rd)
{
    return ((gic_rdist[rd].lpis.gicr_typer[0] >> 8) & 0xFFFF);
}

u32 get_max_lpi(u32 rd)
{
    u32 max_lpi;

    if (!(gic_rdist[rd].lpis.gicr_typer[0] & 0x1))
        return 0;

    max_lpi = ((gic_dist->gicd_typer >> 19) & 0x1F) + 1;
    max_lpi = (1 << max_lpi) - 1;

    return max_lpi;
}


u32 configure_lpi(u32 rd, u32 id, u32 enable, u32 priority)
{
    u8* config;
    u32 max_lpi;

    max_lpi = ((gic_dist->gicd_typer >> 19) & 0x1F) + 1;
    max_lpi = (1 << max_lpi) - 1;

    if ((id < 8192) || (id > max_lpi))
        return 1;

    max_lpi = (gic_rdist[rd].lpis.gicr_propbaser & 0x1F) + 1;
    max_lpi = (1 << max_lpi) - 1;

    if (id > max_lpi)
        return 1;

    config = (u8*)(gic_rdist[rd].lpis.gicr_propbaser & 0x0000FFFFFFFFF000);
    enable = enable & 0x1;
    priority = priority & 0x7C;

    config[(id - 8192)] = (0x2 | enable | priority);
    mb();

#ifdef DIRECT
    gic_rdist[rd].lpis.gicr_invlpir = id;

    while (gic_rdist[rd].lpis.gicr_syncr)
    {}
#endif

    return 0;
}


u32 init_its_cmd_queue(u64 addr, u64 attributes, u32 num_pages)
{
    u64 tmp;

    addr = addr & (u64)0x0000FFFFFFFFF000;
    attributes = attributes & (u64)0xF800000000000C00;
    num_pages  = num_pages  & 0x00000000000000FF;

    if (!num_pages)
        return 1;

    if (num_pages > 256)
        return 1;

    tmp = num_pages * 4096;
    memset((void*)addr, 0, tmp);

    tmp = addr | attributes | (u64)(num_pages - 1);

    gic_its->gits_cbaser  = tmp;
    gic_its->gits_cwriter = 0;

    mb();

    return 0;
}

u64 get_its_table_type(u32 index, u32* type, u32* entry_size)
{
    if (index > 7)
        return 0;

    *type = (u32)(0x7 & (gic_its->gits_baser[index] >> 56));
    *entry_size = (u32)(0x1F & (gic_its->gits_baser[index] >> 48));

    return 1;
}

u32 set_its_table_addr(u32 index, u64 addr, u64 attributes,
        u32 page_size, u32 num_pages)
{
    u64 tmp;

    if (index > 7)
        return 1;

    if (!num_pages)
        return 1;

    tmp = (num_pages - 1) & 0xFF;
    tmp = tmp | (page_size  & 0x300);
    tmp = tmp | (addr & (u64)0x0000FFFFFFFFF000);
    tmp = tmp | (attributes & (u64)0xF800000000000C00);

    gic_its->gits_baser[index] = tmp;

    if (page_size == GICV3_ITS_TABLE_PAGE_SIZE_4K)
        tmp = 0x1000 * num_pages;
    else if (page_size == GICV3_ITS_TABLE_PAGE_SIZE_16K)
        tmp = 0x4000 * num_pages;
    else if (page_size == GICV3_ITS_TABLE_PAGE_SIZE_64K)
        tmp = 0x10000 * num_pages;

    memset((void*)addr, 0, tmp);

    return 0;
}

u32 get_its_pta(void)
{
    return ((gic_its->gits_typer >> 19) & 1);
}

void enable_its(void)
{
    gic_its->gits_ctlr = 1;
}

void disablei_its(void)
{
    gic_its->gits_ctlr = 0;

    while(!(gic_its->gits_ctlr & (1 << 31)))
    {}
}

#define COMMAND_SIZE  (32)

void its_add_cmd(u8* command)
{
    u32 i, queue_size;
    u64 new_cwriter, queue_base, queue_offset, queue_read;
    u8* entry;

    queue_size = ((gic_its->gits_cbaser & 0xFF) + 1) * 0x1000;
    queue_base = (gic_its->gits_cbaser & (u64)0x0000FFFFFFFFF000);
    queue_offset = gic_its->gits_cwriter;
    queue_read = gic_its->gits_creadr;

    if (!queue_read)
        queue_read = queue_size - COMMAND_SIZE;
    else
        queue_read = queue_read - COMMAND_SIZE;

    while (queue_offset == queue_read)
    {}

    entry = (u8*)(queue_base + queue_offset);

    for(i = 0; i < 32; i++)
        entry[i] = command[i];

    mb();

    new_cwriter = queue_offset + COMMAND_SIZE;

    if (new_cwriter == queue_size)
        new_cwriter = 0;

    gic_its->gits_cwriter = new_cwriter;

    while(gic_its->gits_cwriter != gic_its->gits_creadr)
    {}
}

void its_mapd(u32 device_id, u64 table, u32 size)
{
    u8 command[32];
    u32 i;

    if (size > 0)
        size--;
    else
        return;

    memset(command, 0, 32);

    table = table >> 8;

    command[0] = 0x8;
    command[1] = 0x0;

    command[4] = (u8)(0xFF & device_id);
    command[5] = (u8)(0xFF & (device_id >> 8));
    command[6] = (u8)(0xFF & (device_id >> 16));
    command[7] = (u8)(0xFF & (device_id >> 24));

    command[8] = (u8)(0x3F & size);

    command[17] = (u8)(0xFF & table);
    command[18] = (u8)(0xFF & (table >> 8));
    command[19] = (u8)(0xFF & (table >> 16));
    command[20] = (u8)(0xFF & (table >> 24));
    command[21] = (u8)(0xFF & (table >> 32));

    command[23] = 0x80;

    its_add_cmd(command);
}

void its_mapc(u32 target, u32 collection)
{
    u8 command[32];
    u32 i;

    memset(command, 0, 32);

    if ((gic_its->gits_typer & (1 << 19)) != 0)
        target = target >> 16;

    command[0] = 0x9;
    command[1] = 0x0;

    command[16] = (u8)(0xFF & collection);
    command[17] = (u8)(0xFF & (collection >> 8));

    command[18] = (u8)(0xFF & target);
    command[19] = (u8)(0xFF & (target >> 8));
    command[20] = (u8)(0xFF & (target >> 16));
    command[21] = (u8)(0xFF & (target >> 24));

    command[23] = 0x80;

    its_add_cmd(command);
}

void its_mapti(u32 device_id, u32 event_id, u32 intid, u32 cid)
{
    u8 command[32];
    u32 i;

    memset(command, 0, 32);

    command[0] = 0xA;
    command[1] = 0x0;

    command[4] = (u8)(0xFF & device_id);
    command[5] = (u8)(0xFF & (device_id >> 8));
    command[6] = (u8)(0xFF & (device_id >> 16));
    command[7] = (u8)(0xFF & (device_id >> 24));

    command[8] = (u8)(0xFF & event_id);
    command[9] = (u8)(0xFF & (event_id >> 8));
    command[10] = (u8)(0xFF & (event_id >> 16));
    command[11] = (u8)(0xFF & (event_id >> 24));

    command[12] = (u8)(0xFF & intid);
    command[13] = (u8)(0xFF & (intid >> 8));
    command[14] = (u8)(0xFF & (intid >> 16));
    command[15] = (u8)(0xFF & (intid >> 24));

    command[16] = (u8)(0xFF & cid);
    command[17] = (u8)(0xFF & (cid >> 8));

    its_add_cmd(command);
}

void its_invall(u32 cid)
{
    u8 command[32];
    u32 i;

    memset(command, 0, 32);

    command[0] = 0xD;
    command[1] = 0x0;

    command[16] = (u8)(0xFF & cid);
    command[17] = (u8)(0xFF & (cid >> 8));

    its_add_cmd(command);
}

void its_inv(u32 device_id, u32 event_id)
{
    u8 command[32];
    u32 i;

    memset(command, 0, 32);

    command[0] = 0xC;
    command[1] = 0x0;

    command[4] = (u8)(0xFF & device_id);
    command[5] = (u8)(0xFF & (device_id >> 8));
    command[6] = (u8)(0xFF & (device_id >> 16));
    command[7] = (u8)(0xFF & (device_id >> 24));

    command[8] = (u8)(0xFF & event_id);
    command[9] = (u8)(0xFF & (event_id >> 8));
    command[10] = (u8)(0xFF & (event_id >> 16));
    command[11] = (u8)(0xFF & (event_id >> 24));

    its_add_cmd(command);
}

void its_sync(u64 target)
{
    u8 command[32];
    u32 i;

    memset(command, 0, 32);

    target = target >> 16;

    command[0] = 0x5;
    command[1] = 0x0;

    command[18] = (u8)(0xFF & target);
    command[19] = (u8)(0xFF & (target >> 8));
    command[20] = (u8)(0xFF & (target >> 16));
    command[21] = (u8)(0xFF & (target >> 24));

    its_add_cmd(command);
}

void its_int(u32 device_id, u32 event_id)
{
    u8 command[32];
    u32 i;

    memset(command, 0, 32);

    command[0] = 0x3;
    command[1] = 0x0;

    command[4] = (u8)(0xFF & device_id);
    command[5] = (u8)(0xFF & (device_id >> 8));
    command[6] = (u8)(0xFF & (device_id >> 16));
    command[7] = (u8)(0xFF & (device_id >> 24));

    command[8] = (u8)(0xFF & event_id);
    command[9] = (u8)(0xFF & (event_id >> 8));
    command[10] = (u8)(0xFF & (event_id >> 16));
    command[11] = (u8)(0xFF & (event_id >> 24));

    its_add_cmd(command);
}

/* GICv3 Implementation from ARM
 * --------------------------------------------------------------
 *
 */

// --------------------------------------------------------------
void cpu_gicv3_setup(void)
{

}
// --------------------------------------------------------------
