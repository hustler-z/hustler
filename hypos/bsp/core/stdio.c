/**
 * Hustler's Project
 *
 * File:  stdio.c
 * Date:  2024/06/05
 * Usage:
 */

#include <org/globl.h>
#include <bsp/sdev.h>
#include <bsp/hypmem.h>
#include <bsp/serial.h>
#include <bsp/keyboard.h>
#include <bsp/errno.h>
#include <lib/strops.h>

// --------------------------------------------------------------
static struct stdio_dev devs;
struct stdio_dev *stdio_devices[] = { NULL, NULL, NULL };
char *stdio_names[MAX_FILES] = { "stdin", "stdout", "stderr" };

int stdio_file_to_flags(const int file)
{
    switch (file) {
    case stdin:
        return DEV_FLAGS_INPUT;
    case stdout:
    case stderr:
        return DEV_FLAGS_OUTPUT;
    default:
        return -EINVAL;
    }
}

static void stdio_serial_putc(struct stdio_dev *dev, const char c)
{
    serial_putc(c);
}

static void stdio_serial_puts(struct stdio_dev *dev, const char *s)
{
    serial_puts(s);
}

static void stdio_serial_flush(struct stdio_dev *dev)
{
    serial_flush();
}

static int stdio_serial_getc(struct stdio_dev *dev)
{
    return serial_getc();
}

static int stdio_serial_tstc(struct stdio_dev *dev)
{
    return serial_tstc();
}

static void drv_system_init(void)
{
    struct stdio_dev dev;

    memset(&dev, 0, sizeof (dev));

    strcpy(dev.name, "serial");
    dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT;
    dev.putc = stdio_serial_putc;
    dev.puts = stdio_serial_puts;
    STDIO_DEV_ASSIGN_FLUSH(&dev, stdio_serial_flush);
    dev.getc = stdio_serial_getc;
    dev.tstc = stdio_serial_tstc;
    stdio_register (&dev);
}

struct list_head* stdio_get_list(void)
{
    return &devs.list;
}

struct stdio_dev *stdio_get_by_name(const char *name)
{
    struct list_head *pos;
    struct stdio_dev *sdev;

    if (!name)
        return NULL;

    list_for_each(pos, &devs.list) {
        sdev = list_entry(pos, struct stdio_dev, list);
        if (!strcmp(sdev->name, name))
            return sdev;
    }

    return NULL;
}

struct stdio_dev *stdio_clone(struct stdio_dev *dev)
{
    struct stdio_dev *_dev;

    if (!dev)
        return NULL;

    _dev = calloc(1, sizeof(struct stdio_dev));
    if (!_dev)
        return NULL;

    memcpy(_dev, dev, sizeof(struct stdio_dev));

    return _dev;
}

int stdio_register_dev(struct stdio_dev *dev, struct stdio_dev **devp)
{
    struct stdio_dev *_dev;

    _dev = stdio_clone(dev);
    if (!_dev)
        return -ENODEV;
    list_add_tail(&_dev->list, &devs.list);
    if (devp)
        *devp = _dev;

    return 0;
}

int stdio_register(struct stdio_dev *dev)
{
    return stdio_register_dev(dev, NULL);
}

int stdio_deregister_dev(struct stdio_dev *dev, int force)
{
    struct list_head *pos;
    char temp_names[3][16];
    int i;

    /* get stdio hypos_devices (ListRemoveItem changes the dev list) */
    for (i = 0 ; i < MAX_FILES; i++) {
        if (stdio_devices[i] == dev) {
            if (force) {
                strcpy(temp_names[i], "nulldev");
                continue;
            }
            /* Device is assigned -> report error */
            return -EBUSY;
        }
        memcpy(&temp_names[i][0], stdio_devices[i]->name,
               sizeof(temp_names[i]));
    }

    list_del(&dev->list);
    free(dev);

    /* reassign hypos_device list */
    list_for_each(pos, &devs.list) {
        dev = list_entry(pos, struct stdio_dev, list);
        for (i = 0 ; i < MAX_FILES; i++) {
            if (strcmp(dev->name, temp_names[i]) == 0)
                stdio_devices[i] = dev;
        }
    }

    return 0;
}

int stdio_init_tables(void)
{
    /* Initialize the list */
    INIT_LIST_HEAD(&devs.list);

    return 0;
}

int stdio_add_devices(void)
{
    struct hypos_device *dev;
    int ret;

    if (hypos_get(keyboard_enable)) {
        /*
         * For now we probe all the hypos_devices here. At some point this
         * should be done only when the hypos_devices are required - e.g. we
         * have a list of input hypos_devices to start up in the stdin
         * environment variable. That work probably makes more sense
         * when stdio itself is converted to driver model.
         */
        keyboard_setup();
    }

    drv_system_init();
    serial_stdio_init();

    return 0;
}
// --------------------------------------------------------------
