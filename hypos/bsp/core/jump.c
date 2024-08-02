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
#define FUNCJMP(impl, rt, func, ...)  hypos_get(fjmp)->func = impl,

struct funcjmp hypos_fjmp = {
#include <org/jmpglb.h>
};

int jump_tbl_setup(void)
{
    if (get_globl())
        hypos_get(fjmp) = &hypos_fjmp;
}
// --------------------------------------------------------------
