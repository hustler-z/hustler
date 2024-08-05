/**
 * Hustler's Project
 *
 * File:  syms.c
 * Date:  2024/07/03
 * Usage:
 */

#include <bsp/symtbl.h>
#include <bsp/type.h>
#include <asm/hvm.h>

#ifdef SYMBOLS_ORIGIN
const unsigned int symbols_offsets[1];
#else
const unsigned int symbols_addresses[1];
#endif
const unsigned int symbols_num_syms;
const u8 symbols_names[1];
const struct symbol_offset symbols_sorted_offsets[1];
const u8 symbols_token_table[1];
const u16 symbols_token_index[1];
const unsigned int symbols_markers[1];