/**
 * Hustler's Project
 *
 * File:  size.h
 * Date:  2024/07/10
 * Usage:
 */

#ifndef _BSP_SIZE_H
#define _BSP_SIZE_H
// --------------------------------------------------------------
#include <org/bitops.h>

#define KB(_kb)                    (_AC(_kb, UL) << 10)
#define MB(_mb)                    (_AC(_mb, UL) << 20)
#define GB(_gb)                    (_AC(_gb, UL) << 30)

// --------------------------------------------------------------
#endif /* _BSP_SIZE_H */
