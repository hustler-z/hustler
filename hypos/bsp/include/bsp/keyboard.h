/**
 * Hustler's Project
 *
 * File:  keyboard.h
 * Date:  2024/06/07
 * Usage:
 */

/**
 * Hustler's Project
 *
 * File:  input.h
 * Date:  2024/06/07
 * Usage:
 */

#ifndef _BSP_KEYBOARD_H
#define _BSP_KEYBOARD_H
// --------------------------------------------------------------
#include <bsp/input.h>
#include <bsp/sdev.h>

struct keyboard_priv {
	struct stdio_dev sdev;
	struct input_config input;
};

/**
 * struct keyboard_ops - keyboard hypos_device operations
 */
struct keyboard_ops {
	int (*start)(struct hypos_device *dev);
	int (*stop)(struct hypos_device *dev);
	int (*tstc)(struct hypos_device *dev);
	int (*getc)(struct hypos_device *dev);
	int (*update_leds)(struct hypos_device *dev, int leds);
};

#define keyboard_get_ops(dev) ((struct keyboard_ops *)(dev)->driver->ops)

// --------------------------------------------------------------
#endif /* _BSP_KEYBOARD_H */
