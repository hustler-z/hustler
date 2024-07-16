/**
 * Hustler's Project
 *
 * File:  serial.h
 * Date:  2024/06/05
 * Usage:
 */

#ifndef _BSP_SERIAL_H
#define _BSP_SERIAL_H
// --------------------------------------------------------------
#include <bsp/type.h>

enum serial_par {
    SERIAL_PAR_NONE,
    SERIAL_PAR_ODD,
    SERIAL_PAR_EVEN
};

#define SERIAL_PAR_SHIFT	0
#define SERIAL_PAR_MASK		(0x03 << SERIAL_PAR_SHIFT)
#define SERIAL_SET_PARITY(parity) \
	((parity << SERIAL_PAR_SHIFT) & SERIAL_PAR_MASK)
#define SERIAL_GET_PARITY(config) \
	((config & SERIAL_PAR_MASK) >> SERIAL_PAR_SHIFT)

enum serial_bits {
    SERIAL_5_BITS,
    SERIAL_6_BITS,
    SERIAL_7_BITS,
    SERIAL_8_BITS
};

#define SERIAL_BITS_SHIFT	2
#define SERIAL_BITS_MASK	(0x3 << SERIAL_BITS_SHIFT)
#define SERIAL_SET_BITS(bits) \
	((bits << SERIAL_BITS_SHIFT) & SERIAL_BITS_MASK)
#define SERIAL_GET_BITS(config) \
	((config & SERIAL_BITS_MASK) >> SERIAL_BITS_SHIFT)

enum serial_stop {
    SERIAL_HALF_STOP,	/* 0.5 stop bit */
    SERIAL_ONE_STOP,	/*   1 stop bit */
    SERIAL_ONE_HALF_STOP,	/* 1.5 stop bit */
    SERIAL_TWO_STOP		/*   2 stop bit */
};

#define SERIAL_STOP_SHIFT	4
#define SERIAL_STOP_MASK	(0x3 << SERIAL_STOP_SHIFT)
#define SERIAL_SET_STOP(stop) \
	((stop << SERIAL_STOP_SHIFT) & SERIAL_STOP_MASK)
#define SERIAL_GET_STOP(config) \
	((config & SERIAL_STOP_MASK) >> SERIAL_STOP_SHIFT)

#define SERIAL_CONFIG(par, bits, stop) \
         (par << SERIAL_PAR_SHIFT | \
          bits << SERIAL_BITS_SHIFT | \
          stop << SERIAL_STOP_SHIFT)

#define SERIAL_DEFAULT_CONFIG \
        (SERIAL_PAR_NONE << SERIAL_PAR_SHIFT | \
         SERIAL_8_BITS << SERIAL_BITS_SHIFT | \
         SERIAL_ONE_STOP << SERIAL_STOP_SHIFT)

struct serial_device {
    char	name[16];
    int	 (*start)(void);
    int	 (*stop)(void);
    void (*setbrg)(void);
    int	 (*getc)(void);
    int	 (*tstc)(void);
    void (*putc)(const char c);
    void (*puts)(const char *s);

    struct serial_device *next;
};

struct serial_device_info {
    unsigned long addr;
    u8 reg_width;
    u8 reg_offset;
    u8 reg_shift;
    unsigned int clock;
    unsigned int baudrate;
};

struct hypos_device;

struct serial_ops {
    int (*setbrg)(struct hypos_device *dev, int baudrate);
    int (*getc)(struct hypos_device *dev);
    int (*putc)(struct hypos_device *dev, const char ch);
    ssize_t (*puts)(struct hypos_device *dev, const char *s, size_t len);
    int (*pending)(struct hypos_device *dev, bool input);
    int (*clear)(struct hypos_device *dev);
    int (*getconfig)(struct hypos_device *dev, unsigned int *serial_config);
    int (*setconfig)(struct hypos_device *dev, unsigned int serial_config);
    int (*getinfo)(struct hypos_device *dev, struct serial_device_info *info);
};

int serial_pr(const char *fmt, ...)
		__attribute__ ((format (__printf__, 1, 2)));

int  serial_init(void);
void serial_setbrg(void);
void serial_putc(const char c);
void serial_puts(const char *s);
int  serial_getc(void);
int  serial_tstc(void);
static inline void serial_flush(void) {}

void serial_register(struct serial_device *dev);
int  serial_assign(const char *name);
int  serial_initialize(void);
void serial_stdio_init(void);
// --------------------------------------------------------------
#endif /* _BSP_SERIAL_H */
