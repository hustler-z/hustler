/**
 * Hustler's Project
 *
 * File:  hypos.lds.S
 * Date:  2024/05/20
 * Usage: linker script for hypos project
 */

#include <asm/ttbl.h>

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH("aarch64")
ENTRY(_head)

/* Program Headers => Segments
 *
 * Describe how the program should be loaded into memory,
 * Use PHDRS to avoid RWE Permission to LOAD segments.
 *
 * Another solution to avoid RWE:
 * LDFLAGS += -Wl,--no-warn-rwx-segments
 */
PHDRS {
    text PT_LOAD FILEHDR PHDRS;
    data PT_LOAD;
}

SECTIONS
{
    . = HYPOS_VIRT_START;

    __hypos_start = .;

    .boot : {
        __boot_start = .;
        *(.boot.head)
        *(.boot.entry)
        __boot_ttbl_start = .;
        *(.boot.ttbl)
        __boot_ttbl_end = .;
        *(.boot.setup)
        *(.boot .boot.*)
        . = ALIGN(8);
        __boot_end = .;
    } : text

    . = ALIGN(PAGE_SIZE);

    .text : {
        __text_start = .;
        *(.text .text.*)
        . = ALIGN(8);
        __text_end = .;
    } : text

    . = ALIGN(PAGE_SIZE);

    .rodata : {
        __rodata_start = .;
        *(.rodata .rodata.*)
        . = ALIGN(8);
        __rodata_end = .;
    } : data

    __hypos_text_end = .;

    . = ALIGN(PAGE_SIZE);

    __hypos_data_start = .;

    .data : {
        __data_start = .;
        *(.data.boot_pgtbl)
        *(.data.init)
        *(.data.read_mostly)
        *(.data.ro_after_init)
        *(.data .data.*)
        . = ALIGN(PAGE_SIZE);
        __data_end = .;
    } : data

    . = ALIGN(PAGE_SIZE);

    /* Store Board Related Infos
     */
    .board : {
        *(board.data)
        . = ALIGN(8);
    } : data

    . = ALIGN(PAGE_SIZE);

    /* Use for Storing Global Device Data
     */
    __hypos_list : {
        KEEP(*(SORT(__hypos_list*)));
        . = ALIGN(8);
    } : data

    . = ALIGN(PAGE_SIZE);

    .bss : {
        __bss_start = .;
        *(.bss.page_aligned)
        __percpu_start = .;
        *(.bss.percpu)
        *(.bss.thread_percpu)
        __percpu_end = .;
        *(.bss .bss.*)
        . = ALIGN(8);
        __bss_end = .;
    } : data

    . = ALIGN(PAGE_SIZE);

    __hypos_end = .;
}
