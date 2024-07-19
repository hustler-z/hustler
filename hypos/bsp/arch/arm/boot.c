/**
 * Hustler's Project
 *
 * File:  boot.c
 * Date:  2024/05/20
 * Usage:
 */

#include <asm/setup.h>
#include <asm/hvm.h>
#include <org/globl.h>
#include <org/section.h>
#include <org/cache.h>
#include <bsp/debug.h>

// --------------------------------------------------------------
static unsigned char __initdata boot_cpu_stack[STACK_SIZE]
    __attribute__((__aligned__(STACK_SIZE)));

struct boot_setup boot_setup = {
    .stack = boot_cpu_stack,
};

void section_map_dump(void)
{
    MSGI("[globl] Section map dump:\n"
         BLANK_ALIGN"----------------------------------------------------------\n"
         BLANK_ALIGN"<TEXT>  R E  [%016lx - %016lx]  %3lu KB\n"
         BLANK_ALIGN"<DATA>  RWE  [%016lx - %016lx]  %3lu KB\n"
         BLANK_ALIGN"----------------------------------------------------------\n",
        (unsigned long)__hypos_start,
        (unsigned long)__hypos_text_end,
        text_section_size() / KB(1),
        (unsigned long)__hypos_data_start,
        (unsigned long)__hypos_end,
        data_section_size() / KB(1));
}

size_t __read_mostly dcache_line_bytes;

int __bootfunc hcpu_setup(void)
{
    MSGQ(true, "Note phys_offset - 0x%016lx\n",
            hypos_get(phys_offset));

    dcache_line_bytes = read_dcache_line_bytes();

    section_map_dump();

    return 0;
}
// --------------------------------------------------------------
