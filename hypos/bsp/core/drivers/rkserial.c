/**
 * Hustler's Project
 *
 * File:  serial.c
 * Date:  2024/05/20
 * Usage: board debug serial initialization
 */

#include <asm/debug.h>
#include <rockchip/hardware.h>
#include <rk3568/grf.h>
#include <org/bitops.h>
#include <bsp/ns16550.h>
#include <bsp/mmap.h>
// --------------------------------------------------------------

struct rockchip_uart_plat {
    struct ns16550_plat plat;
};

#ifdef __RK3568__
#include <rk3568/rk3568.h>
#include <rk3568/cru.h>
#endif

static int rockchip_serial_probe(struct hypos_device *dev)
{
    struct rockchip_uart_plat *plat = dev_get_plat(dev);

#ifdef __RK3568__
    plat->plat.base = (unsigned long)map_devmem(UART2_BASE);
    plat->plat.clock = OSC_HZ;
    plat->plat.fcr = UART_FCR_DEFVAL;
#endif

    dev_set_plat(dev, &plat->plat);

    return ns16550_serial_probe(dev);
}

HYPOS_SET_DRIVER(rockchip_uart) = {
    .name = "rockchip_uart",
    .enabled = HYP_DRV_ENABLED,
    .type = HYP_DT_SERIAL,
    .probe = rockchip_serial_probe,
    .ops = &ns16550_serial_ops,
};
// --------------------------------------------------------------
