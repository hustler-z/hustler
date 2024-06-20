/**
 * Hustler's Project
 *
 * File:  bitops.h
 * Date:  2024/06/19
 * Usage:
 */

#ifndef _LIB_BITOPS_H
#define _LIB_BITOPS_H
// ------------------------------------------------------------------------
unsigned long find_next_bit(const unsigned long *addr,
                            unsigned long size,
                            unsigned long offset);
unsigned long find_next_zero_bit(const unsigned long *addr,
                                 unsigned long size,
                                 unsigned long offset);
unsigned long find_first_bit(const unsigned long *addr,
                             unsigned long size);
unsigned long find_first_zero_bit(const unsigned long *addr,
                                  unsigned long size);
// ------------------------------------------------------------------------
#endif /* _LIB_BITOPS_H */
