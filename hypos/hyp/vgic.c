/**
 * Hustler's Project
 *
 * File:  vgic.c
 * Date:  2024/05/22
 * Usage:
 */

#include <hyp/vgic.h>
#include <generic/gicv3.h>
#include <asm/barrier.h>
// --------------------------------------------------------------
extern struct gicv3_its_ctlr_if *gic_its;
extern struct gicv3_its_int_if  *gic_its_ints;
extern struct gicv3_dist_if     *gic_dist;
extern struct gicv3_rdist_if    *gic_rdist;
struct gicv3_its_sgi_if         *gic_its_sgi;

// --------------------------------------------------------------
void set_sgi_base(void)
{
    gic_its_sgi = (struct gicv3_its_sgi_if *)((u64)gic_its + 0x20000);
}

u32 is_gicv4x(u32 rd)
{
    if (!((gic_rdist[rd].lpis.gicr_typer[0] >> 1) & 0x1))
        return GICV3_v3X;

    if (!((gic_rdist[rd].lpis.gicr_typer[0] >> 7) & 0x1))
        return GICV3_v40;

    return GICV3_v41;
}

u32 has_vsgi(u32 rd)
{
    u32 its_type, rd_type;

    its_type = (gic_its->gits_typer >> 39) & 0x1;
    rd_type  = (gic_rdist[rd].lpis.gicr_typer[0] >> 26) & 0x1;

    return (its_type & rd_type);
}

u32 set_vpe_conf_table_addr(u32 rd, u64 addr, u64 attributes,
        u32 num_pages)
{
    u64 tmp;
    addr = addr & (u64)0x0000FFFFFFFFF000;
    num_pages = num_pages & 0x000000000000007F;

    if (!num_pages)
        return 1;

    if (num_pages > 127)
        return 1;

    tmp = num_pages * 4096;
    memset((void*)addr, 0, tmp);
    tmp = addr | (num_pages - 1) | ((u64)1 << 52) | (u64)1 << 63;
    gic_rdist[rd].vlpis.gicr_vpropbaser = tmp;

    return 0;
}

u32 make_resident(u32 rd, u32 vpeid, u32 g0, u32 g1)
{
    if ((gic_rdist[rd].vlpis.gicr_vpendbaser & ((u64)1 << 63)))
        return 0xFFFFFFFF;

    gic_rdist[rd].vlpis.gicr_vpendbaser = ((u64)vpeid & 0xFFFF) |
                                        ((u64)1 << 63) |
                                        ((u64)(g1 & 0x1) << 59) |
                                        ((u64)(g0 & 0x1) << 58);

    while ((gic_rdist[rd].vlpis.gicr_vpendbaser & ((u64)0x1 << 60)))
    {}

    return 0;
}


u32 make_not_resident(u32 rd, u32 db)
{
    gic_rdist[rd].vlpis.gicr_vpendbaser = ((u64)(db & 0x1) << 62);

    while ((gic_rdist[rd].vlpis.gicr_vpendbaser & ((u64)0x1 << 60)))
    {}

    return (gic_rdist[rd].vlpis.gicr_vpendbaser & ((u64)0x1 << 61));
}


u32 configure_vlpi(u8* table, u32 id, u32 enable, u32 priority)
{
    u8* config;

    if (id < 8192)
        return 1;

    enable = enable & 0x1;
    priority = priority & 0x7C;

    table[(id - 8192)] = (0x2 | enable | priority);
    mb();

    return 0;
}

u32 its_shared_table_support(void)
{
    return ((gic_its->gits_typer >> 41) & 0x3);
}

u32 its_get_affinity(void)
{
    return gic_its->gits_mpidr;
}

void its_send_sgi(u32 vintid, u32 vpeid)
{
    gic_its_sgi->gits_sgir = (u64)(vintid & 0xF) |
        ((u64)(vpeid & 0xFF) << 32);
}

void its_vmapp(u32 vpeid, u32 target, u64 conf_addr, u64 pend_addr,
        u32 alloc, u32 v, u32 doorbell, u32 size)
{
    u8 command[32];
    u32 i;

    memset(command, 0, 32);

    if ((gic_its->gits_typer & (1 << 19)) != 0)
        target = target >> 16;

    command[0] = 0x29;
    command[1] = alloc | 0x2;

    command[2] = (u8)(0xFF & (conf_addr >> 16));
    command[3] = (u8)(0xFF & (conf_addr >> 24));
    command[4] = (u8)(0xFF & (conf_addr >> 32));
    command[5] = (u8)(0xFF & (conf_addr >> 40));
    command[6] = (u8)(0xFF & (conf_addr >> 48));

    command[8] = (u8)(0xFF & doorbell);
    command[9] = (u8)(0xFF & (doorbell >> 8));
    command[10] = (u8)(0xFF & (doorbell >> 16));
    command[11] = (u8)(0xFF & (doorbell >> 24));

    command[12] = (u8)(0xFF & vpeid);
    command[13] = (u8)(0xFF & (vpeid >> 8));

    command[18] = (u8)(0xFF & target);
    command[19] = (u8)(0xFF & (target >> 8));
    command[20] = (u8)(0xFF & (target >> 16));
    command[21] = (u8)(0xFF & (target >> 24));
    command[23] = (u8)(v << 7);

    command[24] = size;
    command[26] = (u8)(0xFF & (pend_addr >> 16));
    command[27] = (u8)(0xFF & (pend_addr >> 24));
    command[28] = (u8)(0xFF & (pend_addr >> 32));
    command[29] = (u8)(0xFF & (pend_addr >> 40));
    command[30] = (u8)(0xFF & (pend_addr >> 48));

    its_add_cmd(command);
}

void its_vsync(u32 vpeid)
{
    u8 command[32];
    u32 i;

    memset(command, 0, 32);

    command[0]   = 0x25;

    command[12] = (u8)(0xFF & vpeid);
    command[13] = (u8)(0xFF & (vpeid >> 8));

    its_add_cmd(command);
}

void its_vmapti(u32 deviceid, u32 eventid, u32 doorbell,
        u32 vpeid, u32 vintid)
{
    u8 command[32];
    u32 i;

    memset(command, 0, 32);

    command[0]   = 0x2A;

    command[4] = (u8)(0xFF & deviceid);
    command[5] = (u8)(0xFF & (deviceid >> 8));
    command[6] = (u8)(0xFF & (deviceid >> 16));
    command[7] = (u8)(0xFF & (deviceid >> 24));

    command[8] = (u8)(0xFF & eventid);
    command[9] = (u8)(0xFF & (eventid >> 8));
    command[10] = (u8)(0xFF & (eventid >> 16));
    command[11] = (u8)(0xFF & (eventid >> 24));

    command[12] = (u8)(0xFF & vpeid);
    command[13] = (u8)(0xFF & (vpeid >> 8));

    command[16] = (u8)(0xFF & vintid);
    command[17] = (u8)(0xFF & (vintid >> 8));
    command[18] = (u8)(0xFF & (vintid >> 16));
    command[19] = (u8)(0xFF & (vintid >> 24));

    command[20] = (u8)(0xFF & doorbell);
    command[21] = (u8)(0xFF & (doorbell >> 8));
    command[22] = (u8)(0xFF & (doorbell >> 16));
    command[23] = (u8)(0xFF & (doorbell >> 24));

    its_add_cmd(command);
}

void its_invdb(u32 vpeid)
{
    u8 command[32];
    u32 i;

    memset(command, 0, 32);

    command[0]   = 0x2E;

    command[12] = (u8)(0xFF & vpeid);
    command[13] = (u8)(0xFF & (vpeid >> 8));

    its_add_cmd(command);
}

void its_vsgi(u32 vpeid, u32 vintid, u32 enable,
        u32 priority, u32 group, u32 clear)
{
    u8 command[32];
    u32 i;

    memset(command, 0, 32);

    command[0] = 0x23;

    if (!clear)
        command[1] = (u8)(enable | (group << 2));
    else
        command[1] = 0x2;

    command[2] = (u8)(0xF0 & priority);

    command[4] = vintid;

    command[12] = (u8)(0xFF & vpeid);
    command[13] = (u8)(0xFF & (vpeid >> 8));

    its_add_cmd(command);
}
// --------------------------------------------------------------
