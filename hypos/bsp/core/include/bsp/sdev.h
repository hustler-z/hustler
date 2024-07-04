/**
 * Hustler's Project
 *
 * File:  sdev.h
 * Date:  2024/06/05
 * Usage:
 */

#ifndef _BSP_SDEV_H
#define _BSP_SDEV_H
// --------------------------------------------------------------
#include <bsp/stdio.h>
#include <lib/list.h>

#define DEV_FLAGS_INPUT	 0x00000001	/* Device can be used as input	console */
#define DEV_FLAGS_OUTPUT 0x00000002	/* Device can be used as output console */
#define DEV_FLAGS_SERIAL 0x00000004	/* Device priv is a struct device */

int stdio_file_to_flags(const int file);

/* Device information */
struct stdio_dev {
    int	flags;			    /* Device flags	*/
    int	ext;			    /* Supported extensions	*/
    char name[32];		    /* Device name */

    /* GENERAL functions
     */

    int (*start)(struct stdio_dev *dev); /* To start the device */
    int (*stop)(struct stdio_dev *dev);	 /* To stop the device */

    /* OUTPUT functions
     */

    /* To put a char */
    void (*putc)(struct stdio_dev *dev, const char c);
    /* To put a string (accelerator) */
    void (*puts)(struct stdio_dev *dev, const char *s);

    /* To flush output queue */
    void (*flush)(struct stdio_dev *dev);
    #define STDIO_DEV_ASSIGN_FLUSH(dev, flush_func) \
            ((dev)->flush = (flush_func))

    /* INPUT functions
     */

    /* To test if a char is ready... */
    int (*tstc)(struct stdio_dev *dev);
    int (*getc)(struct stdio_dev *dev);	/* To get that char */

    /* Other functions
     */
    void *priv;			/* Private extensions */
    struct list_head list;
};

int stdio_register(struct stdio_dev *dev);
int stdio_register_dev(struct stdio_dev *dev, struct stdio_dev **devp);
int stdio_init_tables(void);
int stdio_add_devices(void);
int stdio_deregister_dev(struct stdio_dev *dev, int force);
struct list_head *stdio_get_list(void);
struct stdio_dev *stdio_get_by_name(const char *name);
struct stdio_dev *stdio_clone(struct stdio_dev *dev);

int drv_keyboard_init(void);
// --------------------------------------------------------------
#endif /* _BSP_SDEV_H */
