/**
 * Hustler's Project
 *
 * File:  mmio.h
 * Date:  2024/07/17
 * Usage:
 */

#ifndef _ORG_MMIO_H
#define _ORG_MMIO_H
// --------------------------------------------------------------
#include <org/esr.h>
#include <bsp/vcpu.h>
#include <bsp/rwlock.h>

#define MAX_IO_HANDLER  16

enum instr_decode_state
{
    INSTR_ERROR,                /* Error encountered while decoding instr */
    INSTR_VALID,                /* ISS is valid, so no need to decode */
    INSTR_LDR_STR_POSTINDEXING,
    INSTR_CACHE,                /* Cache Maintenance instr */
};

typedef struct
{
    struct hcpu_dabt dabt;
    struct instr_details {
        unsigned long rn:5;
        signed int    imm9:9;
        enum instr_decode_state state;
    } dabt_instr;
    gpa_t gpa;
} mmio_info_t;

enum io_state {
    IO_ABORT,       /* The IO was handled by the helper and led to an abort. */
    IO_HANDLED,     /* The IO was successfully handled by the helper. */
    IO_UNHANDLED,   /* The IO was not handled by the helper. */
    IO_RETRY,       /* Retry the emulation for some reason */
};

typedef int (*mmio_read_t)(struct vcpu *v, mmio_info_t *info,
                           register_t *r, void *priv);
typedef int (*mmio_write_t)(struct vcpu *v, mmio_info_t *info,
                            register_t r, void *priv);

struct mmio_handler_ops {
    mmio_read_t read;
    mmio_write_t write;
};

struct mmio_handler {
    gpa_t  addr;
    size_t size;
    const struct mmio_handler_ops *ops;
    void *priv;
};

struct vmmio {
    unsigned int num_entries;
    unsigned int max_num_entries;
    rwlock_t lock;
    struct mmio_handler *handlers;
};

// --------------------------------------------------------------
#endif /* _ORG_MMIO_H */
