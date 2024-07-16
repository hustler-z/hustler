/**
 * Hustler's Project
 *
 * File:  hypos_device.h
 * Date:  2024/06/05
 * Usage:
 */

#ifndef _BSP_DEVICE_H
#define _BSP_DEVICE_H
// --------------------------------------------------------------

#include <lib/list.h>
#include <bsp/compiler.h>
#include <asm/linker.h>

struct hypos_device;

enum hypos_device_type {
    HYP_DT_SERIAL = 0,
    HYP_DT_TIMER,
    HYP_DT_KEYBRD,
    HYP_DT_NR,
    /* TODO */
};

enum hypos_driver_enable {
    HYP_DRV_DISABLED = 0,
    HYP_DRV_ENABLED,
};

struct hypos_driver {
    int type;
    char *name;
    struct hypos_device *devp;
    const int enabled;
    int (*probe)(struct hypos_device *dev);
    int (*remove)(struct hypos_device *dev);
    int (*bind)(struct hypos_device *dev);
    const void *ops;
};

/* XXX: Simple Device Management
 * -----------------------------------------------------------------
 * A static global hypos_device_table for registration of various
 * devices, show as below:
 *
 * +----------+---------+-------- ~
 * |  serial  |   ...   |
 * +----------+---------+-------- ~
 *                 |
 *          various drivers
 */
struct hypos_device_table {
    int type;
    struct list_head entry;
};

struct hypos_device {
    int type;
    const struct hypos_driver *driver;
    char *name;
    void *priv;
    void *plat;
    struct list_head head;
};

#define HYPOS_SET_DRIVER(__name)                       \
    _entry_declare(struct hypos_driver, __name, hypos_driver)

#define HYPOS_GET_DRIVER(__name)                       \
    _entry_acquire(struct hypos_driver, __name, hypos_driver)

#define HYPOS_REF_DRIVER(__name)                       \
    _entry_reference(struct hypos_driver, __name, hypos_driver)

static inline int hypos_driver_enabled(struct hypos_driver *drv)
{
    return drv->enabled == HYP_DRV_ENABLED;
}

void dev_set_plat(struct hypos_device *dev, void *plat);
void *dev_get_plat(const struct hypos_device *dev);
void *dev_get_priv(const struct hypos_device *dev);
void dev_set_priv(struct hypos_device *dev, void *priv);
int dev_get_type(struct hypos_device *dev);
int device_setup(void);

// --------------------------------------------------------------
#endif /* _BSP_DEVICE_H */
