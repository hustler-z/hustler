/**
 * Hustler's Project
 *
 * File:  serial.c
 * Date:  2024/05/20
 * Usage:
 */

#include <common/errno.h>
#include <bsp/serial.h>
#include <bsp/ns16550.h>
#include <bsp/sdev.h>
#include <lib/strops.h>
// --------------------------------------------------------------
static struct serial_device *serial_devices;
static struct serial_device *serial_current;

void serial_register(struct serial_device *dev)
{
    dev->next = serial_devices;
    serial_devices = dev;
}

int serial_assign(const char *name)
{
    struct serial_device *s;

    for (s = serial_devices; s; s = s->next) {
        if (strcmp(s->name, name))
            continue;
        serial_current = s;
        return 0;
    }

    return -EINVAL;
}

static struct serial_device *get_current(void)
{
    struct serial_device *dev;

    if (!serial_current)
        dev = default_serial_console();
    else
        dev = serial_current;

    return dev;
}

int serial_init(void)
{
    return get_current()->start();
}

void serial_setbrg(void)
{
    get_current()->setbrg();
}

int serial_getc(void)
{
    return get_current()->getc();
}

int serial_tstc(void)
{
    return get_current()->tstc();
}

void serial_putc(const char c)
{
    get_current()->putc(c);
}

void serial_puts(const char *s)
{
    get_current()->puts(s);
}

int serial_initialize(void)
{
    /* TODO some other serial drivers
     */

    ns16550_serial_initialize();

    serial_assign(default_serial_console()->name);

    return 0;
}
// --------------------------------------------------------------
static int serial_stub_start(struct stdio_dev *sdev)
{
	struct serial_device *dev = sdev->priv;

	return dev->start();
}

static int serial_stub_stop(struct stdio_dev *sdev)
{
	struct serial_device *dev = sdev->priv;

	return dev->stop();
}

static void serial_stub_putc(struct stdio_dev *sdev, const char ch)
{
	struct serial_device *dev = sdev->priv;

	dev->putc(ch);
}

static void serial_stub_puts(struct stdio_dev *sdev, const char *str)
{
	struct serial_device *dev = sdev->priv;

	dev->puts(str);
}

static int serial_stub_getc(struct stdio_dev *sdev)
{
	struct serial_device *dev = sdev->priv;

	return dev->getc();
}

static int serial_stub_tstc(struct stdio_dev *sdev)
{
	struct serial_device *dev = sdev->priv;

	return dev->tstc();
}

/**
 * serial_stdio_init() - Register serial ports with STDIO core
 *
 * This function generates a proxy driver for each serial port driver.
 * These proxy drivers then register with the STDIO core, making the
 * serial drivers available as STDIO devices.
 */
void serial_stdio_init(void)
{
	struct stdio_dev dev;
	struct serial_device *s = serial_devices;

	while (s) {
		memset(&dev, 0, sizeof(dev));

		strcpy(dev.name, s->name);
		dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT;

		dev.start = serial_stub_start;
		dev.stop = serial_stub_stop;
		dev.putc = serial_stub_putc;
		dev.puts = serial_stub_puts;
		dev.getc = serial_stub_getc;
		dev.tstc = serial_stub_tstc;
		dev.priv = s;

		stdio_register(&dev);

		s = s->next;
	}
}
// --------------------------------------------------------------
