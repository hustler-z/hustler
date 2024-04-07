#include "inc/generic_timer.h"
#include "inc/utils.h"
#include "inc/irq.h"
#include "inc/time_asm.h"
#include "inc/arch_gic.h"
#include "inc/board.h"
#include "../../peripherals/inc/uart.h"

static uint64_t cntfrq = 0;
static uint64_t TIMER_WAIT = 2;

void set_el2_timer_sec(uint64_t _sec)
{
	TIMER_WAIT = _sec;
}

void timer_handler2(void)
{
	uint64_t ticks, current_cnt;
	// uint32_t value;

	// Disable the timer
	__arch_disable_cnthp();
	// value = __arch_read_cnthp_ctl_el2();

	ticks = TIMER_WAIT * cntfrq;

	current_cnt = __arch_read_cntpct_el0();
	__arch_write_cnthp_cval_el2(current_cnt + ticks);

	// Enable the timer
	__arch_enable_cnthp();
	__arch_enable_irq();
}


void timer_el2_init(void)
{
	// uint64_t current_cnt;

	// Disable the timer
	__arch_disable_cnthp();

	cntfrq = __arch_read_cntfrq_el0();
	// current_cnt = __arch_read_cntpct_el0();

	__arch_enable_cnthp();
	__arch_enable_irq();

	SERIAL_NEWLINE;
}
