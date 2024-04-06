#include "inc/hypervisor.h"

/*
 * vm_control.S > asm_start_vm
 */
extern void asm_enable_hypervisor();


void hypervisor_init()
{
    SERIAL_LINE_N;

    uart_print_string("\n"
        "\t********************************\n"
        "\t**    HUSTLER's HYPERVISOR    **\n"
        "\t********************************\n");

    uart_print_string("It's about to go down ...\n");
    SERIAL_NEWLINE;
    uart_print_string("application: ");
    // uart_print_string(APPLICATION);

    SERIAL_NEWLINE;

    uart_print_string("Exception level: ");
    uart_print_hex(__arch_read_current_el());

    SERIAL_NEWLINE;
    SERIAL_LINE_N;

    /* Configure HCR_El2 register in assembly */
    asm_enable_hypervisor();
    // HYPERVISOR_STATUS.enabled = true;

    HYP_LOG_N("Init Success");
}


/*
 * Initialize gic and register devices.
 */
void hypervisor_set_interrupts(void)
{
    gic_init();

    gic_register_device(TIMER_EL2_IRQ);
    HYP_LOG_N("TIMER2 init");

    gic_register_device(UART_IRQ);
    HYP_LOG_N("UART init");
}


/*
 * Enable devices and start interrupts
 */
void hypervisor_enable_interrupts(void)
{
    HYP_LOG_N("Interrupts enabled");
    SERIAL_LINE_N;

    timer_el2_init();
    uart_enable_interrupts();
}
