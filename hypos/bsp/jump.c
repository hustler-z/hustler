/**
 * Hustler's Project
 *
 * File:  jump.c
 * Date:  2024/05/21
 * Usage: jump table implementation
 */

#include <asm-generic/globl.h>
#include <bsp/jump.h>
// --------------------------------------------------------------
extern struct hypos_globl *glb;

#define FUNCJMP(impl, rt, func, ...)  glb->fjmp->func = impl,

struct funcjmp hypos_fjmp = {
#include <asm-generic/jmpglb.h>
};

int jump_tbl_setup(void)
{
    if (glb_is_initialized())
        glb->fjmp = &hypos_fjmp;
}
// --------------------------------------------------------------
