/**
 * Hustler's Project
 *
 * File:  section.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _ORG_SECTION_H
#define _ORG_SECTION_H
// --------------------------------------------------------------
#include <bsp/compiler.h>

#define __initdata          __section(".data.init")
#define __read_mostly       __section(".data.read_mostly")
#define __bootfunc          __section(".boot.setup")
#define __thread_percpu     __section(".bss.thread_percpu")
#define __ro_after_init     __section(".data.ro_after_init")

extern char __hypos_start[], __hypos_end[];
#define is_core_section(v) ({                      \
    char *__v = (char *)(unsigned long)(v);        \
    (__v >= __hypos_start) && (__v < __hypos_end); \
})

extern char __boot_start[], __boot_end[];
#define is_boot_section(v) ({                      \
    char *__v = (char *)(unsigned long)(v);        \
    (__v >= __boot_start) && (__v < __boot_end);   \
})

extern char __text_start[], __text_end[];
#define is_text_section(v) ({                      \
    char *__v = (char *)(unsigned long)(v);        \
    (__v >= __text_start) && (__v < __text_end);   \
})

extern char __rodata_start[], __rodata_end[];
#define is_rodata_section(v) ({                      \
    char *__v = (char *)(unsigned long)(v);          \
    (__v >= __rodata_start) && (__v < __rodata_end); \
})

extern char __data_start[], __data_end[];
#define is_data_section(v) ({                      \
    char *__v = (char *)(unsigned long)(v);        \
    (__v >= __data_start) && (__v < __data_end);   \
})

extern char __hypos_text_end[], __hypos_data_start[];
static inline unsigned long text_section_size(void)
{
    return (unsigned long)(__hypos_text_end - __hypos_start);
}

static inline unsigned long code_size(void)
{
    return (unsigned long)(__hypos_end - __hypos_start);
}

static inline unsigned long data_section_size(void)
{
    return (unsigned long)(__hypos_end - __hypos_data_start);
}

// --------------------------------------------------------------
#endif /* _ORG_SECTION_H */
