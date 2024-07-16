/**
 * Hustler's Project
 *
 * File:  jump.h
 * Date:  2024/05/21
 * Usage:
 */

#ifndef _BSP_JUMP_H
#define _BSP_JUMP_H
// ------------------------------------------------------------------------

/* Define global function jump if necessary.
 */
#define FUNCJMP(impl, rt, func, ...)       rt (*func)(__VA_ARGS__);

struct funcjmp {
#include <org/jmpglb.h>
};

// ------------------------------------------------------------------------
#endif /* _BSP_JUMP_H */
