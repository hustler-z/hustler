/**
 * Hustler's Project
 *
 * File:  mem.h
 * Date:  2024/06/21
 * Usage:
 */

#ifndef _ASM_GENERIC_CHUNK_H
#define _ASM_GENERIC_CHUNK_H
// --------------------------------------------------------------
#include <asm/ttbl.h>
#include <asm-generic/bitops.h>
#include <common/type.h>

#define NR_MEM_CHUNKS  256

struct memchunk {
    vaddr_t start;
    size_t size;
    unsigned int type;
};

struct memchunks {
    unsigned int nr_chunks;
    struct memchunk chunk[NR_MEM_CHUNKS];
};

struct hchunks {
    struct memchunks nchunks;
    struct memchunks rchunks;
};

int hchunks_setup(void);

#define PAGE_INDEX_SHIFT       PGTBL_LEVEL_SHIFT(2)
#define PAGE_INDEX_COUNT \
    ((1 << PAGE_INDEX_SHIFT) / (ISOLATE_LSB(sizeof(*page_head))))

enum memchunk_type {
    MEMCHUNK_DEFAULT,
    MEMCHUNK_STATIC_HEAP,
    MEMCHUNK_RESERVED,
};
// --------------------------------------------------------------
#endif /* _ASM_GENERIC_CHUNK_H */
