/**
 * Hustler's Project
 *
 * File:  exit.c
 * Date:  2024/05/20
 * Usage:
 */

#include <generic/exit.h>
#include <asm/exit.h>
#include <bsp/debug.h>

// --------------------------------------------------------------

void hang(void) {
    MSGH("------------- [Hypervisor Crashed] -------------\n");

    for (;;)
        ;
}

void do_reboot(void)
{
    arch_cpu_reboot();
}

void do_exit(int exit_code)
{

}
// --------------------------------------------------------------
