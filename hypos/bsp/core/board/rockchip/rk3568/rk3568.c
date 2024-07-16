/**
 * Hustler's Project
 *
 * File:  rk3568.c
 * Date:  2024/05/20
 * Usage:
 */

#include <asm/ttbl.h>
#include <org/bitops.h>
#include <org/section.h>
#include <org/globl.h>
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

/* Radxa Zero 3W - 1G of RAM, WIFI Version, RK3566 Quad-Core CPU.
 * So gotta set up the whole thing carefully, in case some weird
 * craps happen.
 */
#define RAM_MEM_START                    _AT(vaddr_t, 0x00200000)
#define RAM_MEM_END                      _AT(vaddr_t, 0x3fffffff)

static struct hypos_mem_region __initdata data_region = {
    .pa_start  = RAM_MEM_START,
    .va_start  = HYPOS_DATA_VIRT_START,
    .size      = 0,
};

static struct hypos_mem_region __initdata fixmap_region = {
    .pa_start  = 0x0,
    .va_start  = FIXADDR_START,
    .size      = FIXADDR_END - FIXADDR_START,
};

static struct hypos_mem_region __initdata vmap_region = {
    .pa_start  = 0x0,
    .va_start  = FIXADDR_END + PAGE_SIZE,
    .size      = MB(256),
};

static struct hypos_mem_region __initdata bootmem_region = {
    .pa_start  = 0x0,
    .va_start  = FIXADDR_END + (PAGE_SIZE * 2) + MB(256),
    .size      = MB(128),
};

static struct hypos_mem_region __initdata directmap_region = {
    .pa_start  = 0x0,
    .va_start  = FIXADDR_END + (PAGE_SIZE * 3) + MB(256 + 128),
    .size      = MB(512),
};

static struct hypos_ram_region __initdata dram_region = {
    .ram_start = RAM_MEM_START,
    .ram_end   = RAM_MEM_END,
    .nr_pfns   = 0,
};

static struct hypos_mem __initdata radax_zero3w_mem = {
    .dram      = &dram_region,
    .data      = &data_region,
    .fixmap    = &fixmap_region,
    .vmap      = &vmap_region,
    .bootmem   = &bootmem_region,
    .directmap = &directmap_region,
};

struct hypos_board __initdata radax_zero3w = {
    .mem       = &radax_zero3w_mem,
};
// --------------------------------------------------------------
