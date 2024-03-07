#ifndef METAL_SYS__H
#define METAL_SYS__H

#ifdef __cplusplus
extern "C"
{
#endif

#include "finterrupt.h"

#define METAL_INTERNAL

#ifdef METAL_INTERNAL

static inline void sys_irq_enable(unsigned int vector)
{
	InterruptUmask(vector);
}

static inline void sys_irq_disable(unsigned int vector)
{
	InterruptMask(vector);
}

#endif /* METAL_INTERNAL */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_GENERIC_SYS__H__ */
