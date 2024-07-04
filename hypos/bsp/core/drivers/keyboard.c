/**
 * Hustler's Project
 *
 * File:  keyboard.c
 * Date:  2024/06/07
 * Usage:
 */

#include <bsp/keyboard.h>
#include <bsp/hypmem.h>
#include <common/errno.h>
#include <lib/strops.h>
// --------------------------------------------------------------
static int keyboard_start(struct stdio_dev *sdev)
{
    struct hypos_device *dev = sdev->priv;
    struct keyboard_ops *ops = keyboard_get_ops(dev);

    if (ops->start)
        return ops->start(dev);

    return 0;
}

static int keyboard_stop(struct stdio_dev *sdev)
{
    struct hypos_device *dev = sdev->priv;
    struct keyboard_ops *ops = keyboard_get_ops(dev);

    if (ops->stop)
        return ops->stop(dev);

    return 0;
}

static int keyboard_tstc(struct stdio_dev *sdev)
{
    struct hypos_device *dev = sdev->priv;
    struct keyboard_priv *priv = dev_get_priv(dev);
    struct keyboard_ops *ops = keyboard_get_ops(dev);

    /* Just get input to do this for us if we can */
    if (priv->input.dev)
        return input_tstc(&priv->input);
    else if (ops->tstc)
        return ops->tstc(dev);

    return -ENOSYS;
}

static int keyboard_getc(struct stdio_dev *sdev)
{
    struct hypos_device *dev = sdev->priv;
    struct keyboard_priv *priv = dev_get_priv(dev);
    struct keyboard_ops *ops = keyboard_get_ops(dev);

    /* Just get input to do this for us if we can */
    if (priv->input.dev)
        return input_getc(&priv->input);
    else if (ops->getc)
        return ops->getc(dev);

    return -ENOSYS;
}

static int __keyboard_setup(struct hypos_device *dev)
{
    struct keyboard_priv *priv = dev_get_priv(dev);
    struct stdio_dev *sdev = &priv->sdev;
    int ret;

    strlcpy(sdev->name, dev->name, sizeof(sdev->name));
    sdev->flags = DEV_FLAGS_INPUT;
    sdev->getc = keyboard_getc;
    sdev->tstc = keyboard_tstc;
    sdev->start = keyboard_start;
    sdev->stop = keyboard_stop;
    sdev->priv = dev;
    ret = input_init(&priv->input, 0);
    if (ret)
        return ret;

    return 0;
}

int keyboard_setup(void)
{
    struct hypos_device *dev;

    dev = alloc(sizeof(struct hypos_device));
    if (!dev)
        return -ENOMEM;

    return __keyboard_setup(dev);
}
// --------------------------------------------------------------
