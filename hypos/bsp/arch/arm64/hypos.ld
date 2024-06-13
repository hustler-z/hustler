/**
 * Hustler's Project
 *
 * File:  hypos.lds.S
 * Date:  2024/05/20
 * Usage: linker script for hypos project
 */

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH("aarch64")
ENTRY(_head)

/* Program Headers => Segments
 *
 * Describe how the program should be loaded
 * into memory. here use PHDRS to avoid RWE
 * Permission to LOAD segments.
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
    . = 0x00200000;

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

    . = ALIGN(0x1000);

    .text : {
        __text_start = .;
        *(.text .text.*)
        . = ALIGN(8);
        __text_end = .;
    } : text

    . = ALIGN(0x1000);

    .rodata : {
        __rodata_start = .;
        *(.rodata .rodata.*)
        . = ALIGN(8);
        __rodata_end = .;
    } : text

    . = ALIGN(0x1000);

    .data : {
        __data_start = .;
        *(.data.boot_pgtbl)
        *(.data .data.*)
        . = ALIGN(8);
        __data_end = .;
    } : data

    . = ALIGN(0x1000);

    /* Use for storing global data:
     * (a) device drivers
     * (b) some necessities
     */
    __hypos_list : {
        KEEP(*(SORT(__hypos_list*)));
        . = ALIGN(8);
    } : data

    . = ALIGN(0x1000);

    .bss : {
        __bss_start = .;
        *(.bss.percpu)
        *(.bss .bss.*)
        . = ALIGN(8);
        __bss_end = .;
    } : data

    . = ALIGN(0x1000);

    .heap : {
        __heap_start = .;
        *(.heap)
        . = . + 0x1000;
        . = ALIGN(8);
        __heap_end = .;
    }

    . = ALIGN(0x1000);

    .hyp_stack : {
        __hyp_stack_start = .;
        . = . + 0x1000;
        . = ALIGN(8);
        __hyp_stack_end = .;
    }

    .guest_stack : {
        __guest_stack_start = .;
        . = . + 0x1000;
        . = ALIGN(8);
        __guest_stack_end = .;
    }

    __hypos_end = .;
}
