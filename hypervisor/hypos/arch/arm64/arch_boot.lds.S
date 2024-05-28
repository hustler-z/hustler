/**
 * Hustler's Project
 *
 * File:  arch_boot.ld
 * Date:  2024/05/20
 * Usage: linker script for this project
 */

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH("aarch64")
ENTRY(boot)

SECTIONS
{
    . = 0x00200000;

    PROVIDE(_hypos_virt_start = .);

    .text :
    {
        _stext = .;
        *(.entry)
        *(.text.idmap)
        *(.init .init.*)
        *(.exit .exit.*)
        *(.text)
        . = ALIGN(8);
        _etext = .;
    }

    . = ALIGN(4096);

    .data :
    {
        *(.data .data.*)
        . = ALIGN(8);
        _edata = .;
    }

    . = ALIGN(4096);

    .rodata :
    {
        *(.rodata .rodata.*)
        . = ALIGN(8);
        _erodata = .;
    }

    . = ALIGN(4096);

    .bss :
    {
        PROVIDE(_bss_start = .);
        *(.bss)
        . = ALIGN(8);
        _ebss = .;
        PROVIDE(_bss_end = .);
    }

    . = ALIGN(4096);

    .heap :
    {
        PROVIDE(_heap_start = .);
        *(.heap)
        . = . + 4096;
        . = ALIGN(8);
        PROVIDE(_heap_end = .);
    }

    . = ALIGN(4096);

    .hyp_stack :
    {
        PROVIDE(_hyp_stack_start = .);
        . = . + 4096;
        . = ALIGN(8);
        PROVIDE(_hyp_stack_end = .);
    }

    .guest_stack :
    {
        PROVIDE(_guest_stack_start = .);
        . = . + 4096;
        . = ALIGN(8);
        PROVIDE(_guest_stack_end = .);
    }

    PROVIDE(_hypos_virt_end = .);
}
