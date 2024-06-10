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
#include <generic/ccattr.h>
#include <asm/linker.h>

struct hypos_device;

enum hypos_device_type {
    HYP_DT_SERIAL = 0,
    HYP_DT_TIMER,
    HYP_DT_KEYBRD,
    /* TODO */
};

struct hypos_driver {
    int type;
    char *name;
    int (*probe)(struct hypos_device *dev);
    int (*remove)(struct hypos_device *dev);
    int (*bind)(struct hypos_device *dev);
    const void *ops;
};

struct hypos_device {
    int type;
    const struct hypos_driver *driver;
    const char *name;
    void *plat;
    void *priv;
    struct hypos_device *parent;
    void *parent_plat;
    void *parent_priv;
    struct list_head child;
};

struct hypos_driver_info {
    const char *name;
    const void *plat;
};

#define HYPOS_SET_DRIVER(__name)                       \
    _entry_declare(struct hypos_driver, __name, hypos_driver)

#define HYPOS_GET_DRIVER(__name)                       \
    _entry_acquire(struct hypos_driver, __name, hypos_driver)

#define HYPOS_REF_DRIVER(__name)                       \
    _entry_reference(struct hypos_driver, __name, hypos_driver)

void *dev_get_priv(const struct hypos_device *dev);
void dev_set_priv(struct hypos_device *dev, void *priv);
void *dev_get_plat(const struct hypos_device *dev);
void dev_set_plat(struct hypos_device *dev, void *plat);
int dev_get_type(struct hypos_device *dev);

int first_device_check(enum hypos_device_type type,
        struct hypos_device **devp);

int next_device_check(struct hypos_device **devp);

int hypos_device_setup(void);

// --------------------------------------------------------------
#endif /* _BSP_DEVICE_H */
