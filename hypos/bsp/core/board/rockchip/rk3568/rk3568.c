/**
 * Hustler's Project
 *
 * File:  rk3568.c
 * Date:  2024/05/20
 * Usage:
 */

#include <asm/ttbl.h>
#include <asm-generic/bitops.h>
#include <asm-generic/section.h>
#include <rk3568/rk3568.h>
#include <rk3568/grf.h>
#include <rockchip/hardware.h>
#include <bsp/board.h>

// --------------------------------------------------------------
/* PMU_GRF_GPIO0D_IOMUX_L */
enum {
	GPIO0D1_SHIFT		= 4,
	GPIO0D1_MASK		= GENMASK(6, 4),
	GPIO0D1_GPIO		= 0,
	GPIO0D1_UART2_TXM0,

	GPIO0D0_SHIFT		= 0,
	GPIO0D0_MASK		= GENMASK(2, 0),
	GPIO0D0_GPIO		= 0,
	GPIO0D0_UART2_RXM0,
};

/* GRF_IOFUNC_SEL3 */
enum {
	UART2_IO_SEL_SHIFT	= 10,
	UART2_IO_SEL_MASK	= GENMASK(11, 10),
	UART2_IO_SEL_M0		= 0,
};

void __bootfunc board_uart_init(void)
{
    static struct rk3568_pmugrf * const pmugrf =
        (struct rk3568_pmugrf *)PMUGRF_BASE;
    static struct rk3568_grf * const grf =
        (struct rk3568_grf *)GRF_BASE;

    /* UART2 M0 */
    rk_clrsetreg(&grf->iofunc_sel3, UART2_IO_SEL_MASK,
             UART2_IO_SEL_M0 << UART2_IO_SEL_SHIFT);

    /* Switch iomux */
    rk_clrsetreg(&pmugrf->pmu_gpio0d_iomux_l,
             GPIO0D1_MASK | GPIO0D0_MASK,
             GPIO0D1_UART2_TXM0 << GPIO0D1_SHIFT |
             GPIO0D0_UART2_RXM0 << GPIO0D0_SHIFT);
}
// --------------------------------------------------------------
#define RZ3W_RESV_MEM_START         _AT(vaddr_t, 0x00200000)
#define RZ3W_RESV_MEM_END           _AT(vaddr_t, 0x3fffffff)

struct hypos_board __board_data radax_zero3w = {
    .pmem.dram.start      = RZ3W_RESV_MEM_START,
    .pmem.dram.end        = RZ3W_RESV_MEM_END,
    .pmem.dram.size       = RZ3W_RESV_MEM_END - RZ3W_RESV_MEM_START,
    .pmem.flush.start     = 0x0,
    .pmem.flush.size      = 0x0,
    .pmem.flush.end       = 0x0,
    .vmem.data.start      = HYPOS_DATA_VIRT_START,
    .vmem.data.size       = HYPOS_DATA_VIRT_SIZE,
    .vmem.data.end        = HYPOS_DATA_VIRT_START + HYPOS_DATA_VIRT_SIZE,
    .vmem.fixmap.start    = FIXADDR_START,
    .vmem.fixmap.size     = FIXADDR_END - FIXADDR_START,
    .vmem.fixmap.end      = FIXADDR_END,
    .vmem.boot.start      = HYPOS_PGFRAME_START,
    .vmem.boot.size       = HYPOS_PGFRAME_SIZE,
    .vmem.boot.end        = HYPOS_PGFRAME_START + HYPOS_PGFRAME_SIZE,
    .vmem.vmap.start      = HYPOS_VMAP_VIRT_START,
    .vmem.vmap.size       = HYPOS_VMAP_VIRT_SIZE,
    .vmem.vmap.end        = HYPOS_VMAP_VIRT_START + HYPOS_VMAP_VIRT_SIZE,
    .vmem.directmap.start = HYPOS_DIRECTMAP_START,
    .vmem.directmap.size  = HYPOS_DIRECTMAP_SIZE,
    .vmem.directmap.end   = HYPOS_DIRECTMAP_START + HYPOS_DIRECTMAP_SIZE,
    .name = "Radxa Zero 3W",
};
// --------------------------------------------------------------
