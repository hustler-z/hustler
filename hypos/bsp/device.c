/**
 * Hustler's Project
 *
 * File:  hypos_device.c
 * Date:  2024/06/05
 * Usage:
 */

#include <asm-generic/globl.h>
#include <bsp/device.h>
#include <bsp/alloc.h>
#include <generic/errno.h>
#include <generic/type.h>
#include <lib/strops.h>
// --------------------------------------------------------------
extern struct hypos_globl *glb;

static struct hypos_driver_info root_info = {
    .name = "root_driver",
};

// --------------------------------------------------------------
void *dev_get_priv(const struct hypos_device *dev)
{
    if (!dev)
        return NULL;

    return dev->priv;
}

void *dev_get_plat(const struct hypos_device *dev)
{
    if (!dev)
        return NULL;

    return dev->plat;
}

void dev_set_priv(struct hypos_device *dev, void *priv)
{
    dev->priv = priv;
}

void dev_set_plat(struct hypos_device *dev, void *plat)
{
    dev->plat = plat;
}

int dev_get_type(struct hypos_device *dev)
{
    return dev->type;
}
// --------------------------------------------------------------

/* Use hypos_device type id to traverse the hypos_devices
 * to find the first device and next device, and so on.
 */
int first_device_check(enum hypos_device_type type,
        struct hypos_device **devp)
{
    /* TODO */
    return 0;
}

int next_device_check(struct hypos_device **devp)
{
    /* TODO */
    return 0;
}

// --------------------------------------------------------------
struct hypos_driver *lists_driver_lookup_name(const char *name)
{
    struct hypos_driver *drv =
        _entry_start(struct hypos_driver, hypos_driver);
    const int n_ents = _entry_count(struct hypos_driver, hypos_driver);
    struct hypos_driver *entry;

    for (entry = drv; entry != drv + n_ents; entry++) {
        if (!strcmp(name, entry->name))
            return entry;
    }

    /* Not found */
    return NULL;
}

static int hypos_device_bind(struct hypos_device *parent,
        const struct hypos_driver *drv, const char *name,
        void *plat, struct hypos_device **devp)
{
    /* TODO
     */
    return 0;
}

static int hypos_device_bind_by_name(struct hypos_device *parent,
        const struct hypos_driver_info *info, struct hypos_device **devp)
{
    struct hypos_driver *drv;
    int ret;

    drv = lists_driver_lookup_name(info->name);
    ret = hypos_device_bind(parent, drv, info->name,
            (void *)info->plat, devp);
    if (ret)
        return ret;

    return ret;
}

static int hypos_device_probe(struct hypos_device *dev)
{
    /* avoid 'const' member
     */
    struct hypos_driver *drv = (struct hypos_driver *)dev->driver;

    if (drv->probe)
        return drv->probe(dev);

    return -1;
}

static int hypos_device_scan(void)
{
    /* TODO
     */
    return 0;
}

int hypos_device_setup(void)
{
    int ret = -1;

    if (!glb_is_initialized())
        return ret;

    ret = hypos_device_bind_by_name(NULL, &root_info,
            &(glb->root_dev));
    if (ret)
        return ret;

    ret = hypos_device_probe(glb->root_dev);
    if (ret)
        return ret;

    ret = hypos_device_scan();
    if (ret)
        return ret;

    return 0;
}
// --------------------------------------------------------------
