/**
 * Hustler's Project
 *
 * File:  boot.c
 * Date:  2024/05/20
 * Usage:
 */

#include <asm/ttbl.h>
#include <asm/exit.h>
#include <asm/hcpu.h>
#include <asm/debug.h>
#include <asm/setup.h>
#include <asm-generic/globl.h>
#include <asm-generic/section.h>
#include <lib/strops.h>
#include <bsp/debug.h>

// --------------------------------------------------------------
static unsigned char __initdata boot_cpu_stack[STACK_SIZE]
    __attribute__((__aligned__(STACK_SIZE)));

struct boot_setup boot_setup = {
    .stack = boot_cpu_stack,
};

void section_map_dump(void)
{
    MSGH("<MAPS> Code Section Dump:\n");
    MSGH("<TEXT> R E [0x%016lx - 0x%016lx] %6lu KB\n",
            (unsigned long)__hypos_start,
            (unsigned long)__hypos_text_end,
            text_section_size() / KB(1));
    MSGH("<DATA> RWE [0x%016lx - 0x%016lx] %6lu KB\n",
            (unsigned long)__hypos_data_start,
            (unsigned long)__hypos_end,
            data_section_size() / KB(1));
}

int __bootfunc hcpu_setup(void)
{
    MSGH("ENV    phys_offset - 0x%016lx\n",
            get_globl()->phys_offset);

    section_map_dump();

    return 0;
}

void arch_cpu_reboot(void)
{
    /* TODO
     */
}
// --------------------------------------------------------------
