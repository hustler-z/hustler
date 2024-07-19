/**
 * Hustler's Project
 *
 * File:  ns16550.c
 * Date:  2024/05/20
 * Usage: board debug serial initialization
 */

#include <org/globl.h>
#include <asm/io.h>
#include <bsp/ns16550.h>
#include <bsp/serial.h>
#include <bsp/period.h>
#include <bsp/type.h>
#include <bsp/errno.h>
#include <lib/math.h>

// --------------------------------------------------------------
#define SYS_NS16550_COM2 0xFE660000
#define SYS_NS16550_CLK  0x0
#define SYS_NS16550_IER  0x0

static int com_idx = 2;
// --------------------------------------------------------------

static struct ns16550 *serial_ports[] = {
    NULL,
    (struct ns16550 *)SYS_NS16550_COM2,
    NULL,
};

#define PORT	serial_ports[port-1]

static void _serial_putc(const char c, const int port)
{
    if (c == '\n')
        ns16550_putc(PORT, '\r');

    ns16550_putc(PORT, c);
}

static void _serial_puts(const char *s, const int port)
{
    while (*s) {
        _serial_putc(*s++, port);
    }
}

static int _serial_getc(const int port)
{
    return ns16550_getc(PORT);
}

static int _serial_tstc(const int port)
{
    return ns16550_tstc(PORT);
}

static void _serial_setbrg(const int port)
{
    int clock_divisor;

    clock_divisor = ns16550_calc_divisor(PORT, SYS_NS16550_CLK,
                         get_globl()->baudrate);
    ns16550_reinit(PORT, clock_divisor);
}

static inline void
serial_putc_dev(unsigned int dev_index,const char c)
{
    _serial_putc(c,dev_index);
}

static inline void
serial_puts_dev(unsigned int dev_index,const char *s)
{
    _serial_puts(s,dev_index);
}

static inline int
serial_getc_dev(unsigned int dev_index)
{
    return _serial_getc(dev_index);
}

static inline int
serial_tstc_dev(unsigned int dev_index)
{
    return _serial_tstc(dev_index);
}

static inline void
serial_setbrg_dev(unsigned int dev_index)
{
    _serial_setbrg(dev_index);
}

#define DECLARE_ESERIAL_FUNCTIONS(port) \
static int  eserial##port##_init(void) \
    { \
        int clock_divisor; \
        clock_divisor = ns16550_calc_divisor(serial_ports[port-1], \
            SYS_NS16550_CLK, get_globl()->baudrate); \
        ns16550_init(serial_ports[port - 1], clock_divisor); \
        return 0 ; \
    } \
    static void eserial##port##_setbrg(void) \
    { \
        serial_setbrg_dev(port); \
    } \
    static int  eserial##port##_getc(void) \
    { \
        return serial_getc_dev(port); \
    } \
    static int  eserial##port##_tstc(void) \
    { \
        return serial_tstc_dev(port); \
    } \
    static void eserial##port##_putc(const char c) \
    { \
        serial_putc_dev(port, c); \
    } \
    static void eserial##port##_puts(const char *s) \
    { \
        serial_puts_dev(port, s); \
    }

/* Serial hypos_device descriptor */
#define INIT_ESERIAL_STRUCTURE(port, __name) {	\
	.name	= __name,			\
	.start	= eserial##port##_init,		\
	.stop	= NULL,				\
	.setbrg	= eserial##port##_setbrg,	\
	.getc	= eserial##port##_getc,		\
	.tstc	= eserial##port##_tstc,		\
	.putc	= eserial##port##_putc,		\
	.puts	= eserial##port##_puts,		\
}

DECLARE_ESERIAL_FUNCTIONS(2);
struct serial_device eserial2_device =
	INIT_ESERIAL_STRUCTURE(2, "eserial1");

void ns16550_serial_initialize(void)
{
    switch (com_idx) {
    case 1:
        /* TODO */
        break;
    case 2:
        serial_register(&eserial2_device);
        break;
    default:
        return;
    }
}

struct serial_device *default_serial_console(void)
{
    switch (com_idx) {
    case 1:
        /* TODO */
        return NULL;
    case 2:
        return &eserial2_device;
    default:
        return NULL;
    }
}

// --------------------------------------------------------------
#define UART_LCRVAL UART_LCR_8N1		/* 8 data, 1 stop, no parity */
#define UART_MCRVAL (UART_MCR_DTR | \
		     UART_MCR_RTS)		/* RTS/DTR */

/* ------------------------------------------------------------------------
 * Note NS16550 SERIAL IMPLEMENTATION
 * ------------------------------------------------------------------------
 */
static inline void serial_out_shift(void *addr, int shift, int value)
{
	writel(value, addr);
}

static inline int serial_in_shift(void *addr, int shift)
{
	return readl(addr);
}
// --------------------------------------------------------------

static void ns16550_writeb(struct ns16550 *port, int offset, int value)
{
	struct ns16550_plat *plat = port->plat;
	unsigned char *addr;

	offset *= 1 << plat->reg_shift;
	addr = (unsigned char *)plat->base + offset + plat->reg_offset;

	serial_out_shift(addr, plat->reg_shift, value);
}

static int ns16550_readb(struct ns16550 *port, int offset)
{
	struct ns16550_plat *plat = port->plat;
	unsigned char *addr;

	offset *= 1 << plat->reg_shift;
	addr = (unsigned char *)plat->base + offset + plat->reg_offset;

	return serial_in_shift(addr, plat->reg_shift);
}

static u32 ns16550_getfcr(struct ns16550 *port)
{
	struct ns16550_plat *plat = port->plat;

	return plat->fcr;
}

/* We can clean these up once everything is moved to driver model */
#define serial_out(value, addr)	\
	ns16550_writeb(com_port, \
		(unsigned char *)addr - (unsigned char *)com_port, value)
#define serial_in(addr) \
	ns16550_readb(com_port, \
		(unsigned char *)addr - (unsigned char *)com_port)
// ------------------------------------------------------------------------

int ns16550_calc_divisor(struct ns16550 *port, int clock, int baudrate)
{
	const unsigned int mode_x_div = 16;

	return DIV_ROUND_CLOSEST(clock, mode_x_div * baudrate);
}

static void ns16550_setbrg(struct ns16550 *com_port, int baud_divisor)
{
	/* to keep serial format, read lcr before writing BKSE */
	int lcr_val = serial_in(&com_port->lcr) & ~UART_LCR_BKSE;

	serial_out(UART_LCR_BKSE | lcr_val, &com_port->lcr);
	serial_out(baud_divisor & 0xff, &com_port->dll);
	serial_out((baud_divisor >> 8) & 0xff, &com_port->dlm);
	serial_out(lcr_val, &com_port->lcr);
}

void ns16550_init(struct ns16550 *com_port, int baud_divisor)
{
	while (!(serial_in(&com_port->lsr) & UART_LSR_TEMT))
		;

	serial_out(SYS_NS16550_IER, &com_port->ier);

	serial_out(UART_MCRVAL, &com_port->mcr);
	serial_out(ns16550_getfcr(com_port), &com_port->fcr);
	/* initialize serial config to 8N1 before writing baudrate */
	serial_out(UART_LCRVAL, &com_port->lcr);
	if (baud_divisor != -1)
		ns16550_setbrg(com_port, baud_divisor);
}

void ns16550_reinit(struct ns16550 *com_port, int baud_divisor)
{
	serial_out(SYS_NS16550_IER, &com_port->ier);
	ns16550_setbrg(com_port, 0);
	serial_out(UART_MCRVAL, &com_port->mcr);
	serial_out(ns16550_getfcr(com_port), &com_port->fcr);
	ns16550_setbrg(com_port, baud_divisor);
}

void ns16550_putc(struct ns16550 *com_port, char c)
{
	while ((serial_in(&com_port->lsr) & UART_LSR_THRE) == 0)
		;
	serial_out(c, &com_port->thr);

	if (c == '\n')
		periodic_work_schedule();
}

char ns16550_getc(struct ns16550 *com_port)
{
	while (!(serial_in(&com_port->lsr) & UART_LSR_DR))
		periodic_work_schedule();

	return serial_in(&com_port->rbr);
}

int ns16550_tstc(struct ns16550 *com_port)
{
	return (serial_in(&com_port->lsr) & UART_LSR_DR) != 0;
}
// ------------------------------------------------------------------------

static int ns16550_serial_putc(struct hypos_device *dev, const char ch)
{
	struct ns16550 *const com_port = dev_get_priv(dev);

	if (!(serial_in(&com_port->lsr) & UART_LSR_THRE))
		return -EAGAIN;
	serial_out(ch, &com_port->thr);

	if (ch == '\n')
		periodic_work_schedule();

	return 0;
}

static int ns16550_serial_pending(struct hypos_device *dev, bool input)
{
	struct ns16550 *const com_port = dev_get_priv(dev);

	if (input)
		return (serial_in(&com_port->lsr) & UART_LSR_DR) ? 1 : 0;
	else
		return (serial_in(&com_port->lsr) & UART_LSR_THRE) ? 0 : 1;
}

static int ns16550_serial_getc(struct hypos_device *dev)
{
	struct ns16550 *const com_port = dev_get_priv(dev);

	if (!(serial_in(&com_port->lsr) & UART_LSR_DR))
		return -EAGAIN;

	return serial_in(&com_port->rbr);
}

static int ns16550_serial_setbrg(struct hypos_device *dev,
        int baudrate)
{
	struct ns16550 *const com_port = dev_get_priv(dev);
	struct ns16550_plat *plat = com_port->plat;
	int clock_divisor;

	clock_divisor = ns16550_calc_divisor(com_port, plat->clock, baudrate);

	ns16550_setbrg(com_port, clock_divisor);

	return 0;
}

static int ns16550_serial_setconfig(struct hypos_device *dev,
        unsigned int serial_config)
{
	struct ns16550 *const com_port = dev_get_priv(dev);
	int lcr_val = UART_LCR_WLS_8;
	unsigned int parity = SERIAL_GET_PARITY(serial_config);
	unsigned int bits = SERIAL_GET_BITS(serial_config);
	unsigned int stop = SERIAL_GET_STOP(serial_config);

	if (bits != SERIAL_8_BITS || stop != SERIAL_ONE_STOP)
		return -ENOTSUP; /* not supported in driver*/

	switch (parity) {
	case SERIAL_PAR_NONE:
		/* no bits to add */
		break;
	case SERIAL_PAR_ODD:
		lcr_val |= UART_LCR_PEN;
		break;
	case SERIAL_PAR_EVEN:
		lcr_val |= UART_LCR_PEN | UART_LCR_EPS;
		break;
	default:
		return -ENOTSUP; /* not supported in driver*/
	}

	serial_out(lcr_val, &com_port->lcr);
	return 0;
}

int ns16550_serial_probe(struct hypos_device *dev)
{
	struct ns16550_plat *plat = dev_get_plat(dev);
	struct ns16550 *const com_port = dev_get_priv(dev);
	int ret;

	com_port->plat = dev_get_plat(dev);
	ns16550_init(com_port, -1);

	return 0;
}

const struct serial_ops ns16550_serial_ops = {
    .putc = ns16550_serial_putc,
    .pending = ns16550_serial_pending,
    .getc = ns16550_serial_getc,
    .setbrg = ns16550_serial_setbrg,
    .setconfig = ns16550_serial_setconfig,
};

HYPOS_SET_DRIVER(ns16550) = {
    .name = "ns16550",
    .enabled = HYP_DRV_DISABLED,
    .type = HYP_DT_SERIAL,
    .probe = ns16550_serial_probe,
    .ops = &ns16550_serial_ops,
};

// ------------------------------------------------------------------------
