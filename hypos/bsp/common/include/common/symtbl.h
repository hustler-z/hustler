/**
 * Hustler's Project
 *
 * File:  systbl.h
 * Date:  2024/06/26
 * Usage:
 */

#ifndef _COMMON_SYMTBL_H
#define _COMMON_SYMTBL_H
// ------------------------------------------------------------------------
#include <common/type.h>
#include <lib/list.h>

#define KSYM_NAME_LEN    127

struct symbol_offset {
    u32 stream;
    u32 addr;
};

typedef const char *symbols_lookup_t(unsigned long addr,
                                     unsigned long *symbolsize,
                                     unsigned long *offset,
                                     char *namebuf);

const char *symbols_lookup(unsigned long addr,
                           unsigned long *symbolsize,
                           unsigned long *offset,
                           char *namebuf);

unsigned long symbols_lookup_by_name(const char *symname);

int syms_read(u32 *symnum, char *type,
              unsigned long *address,
              char *name);

struct vsection {
    struct list_head list;

    const void *text_start;
    const void *text_end;

    const void *rodata_start;
    const void *rodata_end;

    symbols_lookup_t *symbols_lookup;
};
// ------------------------------------------------------------------------
#endif /* _COMMON_SYMTBL_H */
