/**
 * Hustler's Project
 *
 * File:  jump.c
 * Date:  2024/05/21
 * Usage: jump table implementation
 */

#include <org/globl.h>
#include <bsp/jump.h>
// --------------------------------------------------------------
#define FUNCJMP(impl, rt, func, ...)  get_globl()->fjmp->func = impl,

struct funcjmp hypos_fjmp = {
#include <org/jmpglb.h>
};

int jump_tbl_setup(void)
{
    if (get_globl())
        get_globl()->fjmp = &hypos_fjmp;
}
// --------------------------------------------------------------
