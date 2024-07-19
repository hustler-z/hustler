/**
 * Hustler's Project
 *
 * File:  rk3568.c
 * Date:  2024/05/20
 * Usage:
 */

#include <org/bitops.h>
#include <org/section.h>
#include <org/globl.h>
#include <asm/hvm.h>
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
 *
 * Data Region (codes: text, bss, etc.) is fo sure certain.
 */

static struct hypos_cpu __initdata radax_zero3w_cpu = {
    .core_nr = 4,
    .vendor  = VENDOR_ROCKCHIP,
    .priv    = "RK3566 Quad-Core ARM Cortex-A55",
};

static struct hypos_mem __initdata radax_zero3w_mem = {
    /* Host Physical Memory Layout */
    .hpm = {
        [0] = {
            .start = 0x00200000,
            .end   = 0x00A00000,
            .type  = TXT_BLK,
        },

        [1] = {
            .start = 0x00A00000,
            .end   = 0x3EA00000,
            .type  = RAM_BLK,
        },

        [2] = {
            .start = 0x3EA00070,
            .end   = 0x3FFFFFFF,
            .type  = RSV_BLK,
        },
    },

    /* Host Virtual Memory Layout */
    .hvm = {
        [0] = {
            .start = HYPOS_DATA_VIRT_START,
            .size  = HYPOS_DATA_VIRT_SIZE,
            .type  = DATA_BLK,
        },

        [1] = {
            .start = FIXADDR_START,
            .size  = FIXADDR_END - FIXADDR_START,
            .type  = FMAP_BLK,
        },

        [2] = {
            .start = HVM_VMAP_START,
            .size  = HVM_VMAP_SIZE,
            .type  = VMAP_BLK,
        },

        [3] = {
            .start = HVM_BTMB_START,
            .size  = HVM_BTMB_SIZE,
            .type  = BTMB_BLK,
        },

        [4] = {
            .start = HVM_DMAP_START,
            .size  = HVM_DMAP_SIZE,
            .type  = DMAP_BLK,
        },
    }
};

struct hypos_board __initdata radax_zero3w = {
    .mem       = &radax_zero3w_mem,
    .cpu       = &radax_zero3w_cpu,
};
// --------------------------------------------------------------
