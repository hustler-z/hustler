/**
 * Hustler's Project
 *
 * File:  vgic.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _HYP_VGIC_H
#define _HYP_VGIC_H
// ------------------------------------------------------------------------

#include <common/type.h>
#include <lib/strops.h>

void set_sgi_base(void);
u32 is_gicv4x(u32 rd);
u32 has_vsgi(u32 rd);
u32 set_vpe_conf_table_addr(u32 rd, u64 addr, u64 attributes,
        u32 num_pages);
u32 make_resident(u32 rd, u32 vpeid, u32 g0, u32 g1);
u32 make_not_resident(u32 rd, u32 db);
u32 configure_vlpi(u8* table, u32 id, u32 enable, u32 priority);
u32 its_shared_table_support(void);
u32 its_get_affinity(void);
void its_send_sgi(u32 vintid, u32 vpeid);
void its_vmapp(u32 vpeid, u32 target, u64 conf_addr, u64 pend_addr,
        u32 alloc, u32 v, u32 doorbell, u32 size);
void its_vsync(u32 vpeid);
void its_vmapti(u32 deviceid, u32 eventid, u32 doorbell,
        u32 vpeid, u32 vintid);
void its_invdb(u32 vpeid);
void its_vsgi(u32 vpeid, u32 vintid, u32 enable,
        u32 priority, u32 group, u32 clear);

// ------------------------------------------------------------------------
#endif /* _HYP_VGIC_H */
