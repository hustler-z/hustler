/**
 * Hustler's Project
 *
 * File:  jmpglb.h
 * Date:  2024/05/21
 * Usage: Global jump function can be used everywhere
 */

// --------------------------------------------------------------
#ifndef FUNCJMP
#define FUNCJMP(impl, rt, func, ...)
#endif
    FUNCJMP(tstc, int, tstc, void)
    FUNCJMP(putc, void, putc, const char)
    FUNCJMP(puts, void, puts, const char *)
    FUNCJMP(pr, int, pr, const char *, ...)
    FUNCJMP(getc, int, getc, void)
    FUNCJMP(flush, void, flush, void)
// --------------------------------------------------------------
