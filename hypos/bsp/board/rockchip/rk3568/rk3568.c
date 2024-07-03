/**
 * Hustler's Project
 *
 * File:  rk3568.c
 * Date:  2024/05/20
 * Usage:
 */

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

struct hypos_board radax_zero3w = {
    .dram.start  = 0x00200000,
    .dram.end    = 0x3fffffff,
    .flush.start = 0x0,
    .flush.size  = 0x0,
    .name = "Radxa Zero 3W",
};

struct hypos_board *__bootfunc board_setup(void)
{
    struct hypos_board *this = &radax_zero3w;

    this->dram.size = this->dram.end - this->dram.start;

    return this;
}
// --------------------------------------------------------------
