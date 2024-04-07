/*
    Helper functions for GIC implementation
    - registering device
    - initialize gic
*/

#include "inc/irq.h"
#include "inc/arch_gic.h"
#include "../../peripherals/inc/uart.h"
#include "gicv3/gicv3_basic.h"
#include "gicv3/gicv3_registers.h"

static uint32_t GIC_REDIST_ID = 0;

extern uint32_t getAffinity();
extern void setPriorityMask(uint32_t prio_mask);
extern void enableGroup0Ints();
extern void enableGroup1Ints();

void gic_init(void)
{
    setGICAddr((void *)VIRT_GIC_DIST, (void *)VIRT_GIC_REDIST);
    enableGIC();

    GIC_REDIST_ID = getRedistID(getAffinity());
    wakeUpRedist(GIC_REDIST_ID);

    setPriorityMask(0xFF);

    enableGroup0Ints();
    enableGroup1Ints();
}

void gic_register_device(uint64_t _device_id)
{
    setIntPriority(_device_id, GIC_REDIST_ID, 0);
    setIntGroup(_device_id, GIC_REDIST_ID, 0);
    enableInt(_device_id, GIC_REDIST_ID);
}
