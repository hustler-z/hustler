/**
 * Hustler's Project
 *
 * File:  symtbl.c
 * Date:  2024/06/26
 * Usage:
 */

#include <asm/ttbl.h>
#include <asm-generic/section.h>
#include <asm-generic/spinlock.h>
#include <common/symtbl.h>
#include <common/errno.h>
#include <lib/strops.h>
#include <lib/convert.h>

// --------------------------------------------------------------
static struct list_head vsection_list;
static struct vsection hypos_core, hypos_boot;

#define LIST_ENTRY_HEAD() { .next = &hypos_core.list, .prev = &hypos_boot.list }
#define LIST_ENTRY_CORE() { .next = &hypos_boot.list, .prev = &vsection_list }
#define LIST_ENTRY_BOOT() { .next = &vsection_list,   .prev = &hypos_core.list }

static struct vsection hypos_core __read_mostly = {
    .list = LIST_ENTRY_CORE(),
    .text_start = __text_start,
    .text_end = __text_end,
    .rodata_start = __rodata_start,
    .rodata_end = __rodata_end,
};

static struct vsection hypos_boot __initdata = {
    .list = LIST_ENTRY_BOOT(),
    .text_start = __boot_start,
    .text_end = __boot_end,
};

static struct list_head vsection_list = LIST_ENTRY_HEAD();
static DEFINE_SPINLOCK(vsection_lock);

const struct vsection *find_text_section(unsigned long addr)
{
    const struct vsection *iter, *section = NULL;

    spinlock(&vsection_lock);

    list_for_each_entry(iter, &vsection_list, list) {
        if ((void *)addr >= iter->text_start &&
            (void *)addr <  iter->text_end) {
            section = iter;

            break;
        }
    }

    spinunlock(&vsection_lock);

    return section;
}
// --------------------------------------------------------------

const unsigned int symbols_offsets[1];
#define symbols_address(n) (SYMBOLS_ORIGIN + symbols_offsets[n])
const unsigned int symbols_num_syms;
const u8 symbols_names[1];
const struct symbol_offset symbols_sorted_offsets[1];
const u8 symbols_token_table[1];
const u16 symbols_token_index[1];
const unsigned int symbols_markers[1];

// --------------------------------------------------------------

static unsigned int symbols_expand_symbol(unsigned int off, char *result)
{
    int len, skipped_first = 0;
    const u8 *tptr, *data;

    data = &symbols_names[off];
    len = *data;
    data++;

    off += len + 1;

    while (len) {
        tptr = &symbols_token_table[symbols_token_index[*data]];
        data++;
        len--;

        while (*tptr) {
            if(skipped_first) {
                *result = *tptr;
                result++;
            } else
                skipped_first = 1;
            tptr++;
        }
    }

    *result = '\0';

    return off;
}

static unsigned int get_symbol_offset(unsigned long pos)
{
    const u8 *name;
    int i;

    name = &symbols_names[symbols_markers[pos >> 8]];

    for (i = 0; i < (pos & 0xFF); i++)
        name = name + (*name) + 1;

    return name - symbols_names;
}

bool is_active_hypos_text(unsigned long addr)
{
    return !!find_text_section(addr);
}

static char symbols_get_symbol_type(unsigned int off)
{
    return symbols_token_table[symbols_token_index[symbols_names[off + 1]]];
}

static DEFINE_SPINLOCK(sym_lock);

int syms_read(u32 *symnum, char *type,
              unsigned long *address,
              char *name)
{
    static u64 next_symbol, next_offset;


    if (*symnum > symbols_num_syms)
        return -ERANGE;
    if (*symnum == symbols_num_syms) {
        name[0] = '\0';
        return 0;
    }

    spinlock(&sym_lock);

    if (*symnum == 0)
        next_offset = next_symbol = 0;
    if (next_symbol != *symnum)
        next_offset = get_symbol_offset(*symnum);

    *type = symbols_get_symbol_type(next_offset);
    next_offset = symbols_expand_symbol(next_offset, name);
    *address = symbols_address(*symnum);

    next_symbol = ++*symnum;

    spinunlock(&sym_lock);

    return 0;
}

const char *symbols_lookup(unsigned long addr,
                           unsigned long *symbolsize,
                           unsigned long *offset,
                           char *namebuf)
{
    unsigned long i, low, high, mid;
    unsigned long symbol_end = 0;
    const struct vsection *section;

    namebuf[KSYM_NAME_LEN] = 0;
    namebuf[0] = 0;

    section = find_text_section(addr);
    if (!section)
        return NULL;

    if (section->symbols_lookup)
        return section->symbols_lookup(addr, symbolsize, offset, namebuf);

    low = 0;
    high = symbols_num_syms;

    while (high - low > 1) {
        mid = (low + high) / 2;
        if (symbols_address(mid) <= addr)
            low = mid;
        else
            high = mid;
    }

    while (low && symbols_address(low - 1) == symbols_address(low))
        --low;

    symbols_expand_symbol(get_symbol_offset(low), namebuf);

    for (i = low + 1; i < symbols_num_syms; i++) {
        if (symbols_address(i) > symbols_address(low)) {
            symbol_end = symbols_address(i);
            break;
        }
    }

    /* if we found no next symbol, we use the end of the section */
    if (!symbol_end)
        symbol_end = is_boot_section(addr) ?
            (unsigned long)__boot_start : (unsigned long)__text_start;

    *symbolsize = symbol_end - symbols_address(low);
    *offset = addr - symbols_address(low);

    return namebuf;
}

unsigned long symbols_lookup_by_name(const char *symname)
{
    char name[KSYM_NAME_LEN + 1];
    unsigned long low, high;

    if ( *symname == '\0' )
        return 0;

    low = 0;
    high = symbols_num_syms;
    while ( low < high )
    {
        unsigned long mid = low + ((high - low) / 2);
        const struct symbol_offset *s;
        int rc;

        s = &symbols_sorted_offsets[mid];
        (void)symbols_expand_symbol(s->stream, name);

        rc = strcmp(symname, name);
        if ( rc < 0 )
            high = mid;
        else if ( rc > 0 )
            low = mid + 1;
        else
            return symbols_address(s->addr);
    }

    return 0;
}
// --------------------------------------------------------------
