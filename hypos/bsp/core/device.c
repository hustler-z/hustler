/**
 * Hustler's Project
 *
 * File:  hypos_device.c
 * Date:  2024/06/05
 * Usage:
 */

#include <org/globl.h>
#include <bsp/device.h>
#include <bsp/memz.h>
#include <bsp/debug.h>
#include <bsp/errno.h>
#include <bsp/type.h>
#include <lib/strops.h>
// --------------------------------------------------------------
static struct hypos_device_table device_table[HYP_DT_NR];
// --------------------------------------------------------------
void dev_set_priv(struct hypos_device *dev, void *priv)
{
    dev->priv = priv;
}

void *dev_get_priv(const struct hypos_device *dev)
{
    return dev->priv;
}

void dev_set_plat(struct hypos_device *dev, void *plat)
{
    dev->plat = plat;
}

void *dev_get_plat(const struct hypos_device *dev)
{
    return dev->plat;
}

int dev_get_type(struct hypos_device *dev)
{
    return dev->type;
}
// --------------------------------------------------------------
static int hypos_device_bind(struct hypos_driver *drv,
        struct hypos_device_table *table)
{
    struct hypos_device *dev;

    dev = alloc(sizeof(struct hypos_device));
    if (!dev) {
        DEBUG("[dev] allocate device failed!!\n");
        return -ENOMEM;
    }

    dev->driver = drv;
    dev->name = drv->name;

    INIT_LIST_HEAD(&dev->head);

    list_add(&dev->head, &table[drv->type].entry);

    return 0;
}

static int hypos_device_scan_and_probe(void)
{
    struct hypos_driver
        *each,
        *entry = _entry_start(struct hypos_driver, hypos_driver);
    const int n_ents = _entry_count(struct hypos_driver, hypos_driver);
    int idx, ret;

    for (idx = 0; idx < n_ents; idx++) {
        each = entry + idx;
        if (hypos_driver_enabled(each)) {
            ret = hypos_device_bind(each, hypos_get(dev_tbl));
            if (!ret)
                each->probe(each->devp);
        } else {
            DEBUG("[dev] %s has been disabled!!\n",
                    each->name);
            ret = -ENODEV;
        }
    }

    return ret;
}

static void hypos_device_table_setup(struct hypos_device_table *table)
{
    int idx;

    for (idx = 0; idx < HYP_DT_NR; idx++) {
        INIT_LIST_HEAD(&table[idx].entry);
    }

    hypos_get(dev_tbl) = table;
}

int device_setup(void)
{
    int ret = -1;

    if (!hypos_get(flags) & GLB_INITIALIZED)
        return ret;

    hypos_device_table_setup(device_table);

    if (ret)
        return ret;

    ret = hypos_device_scan_and_probe();
    if (ret)
        return ret;

    hypos_get(flags) |= GLB_DEVICE_INIT;

    return 0;
}
// --------------------------------------------------------------
