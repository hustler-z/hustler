/**
 * Hustler's Project
 *
 * File:  cpu_init.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _CPU_INIT_H
#define _CPU_INIT_H
// ------------------------------------------------------------------------
#include <common_ccattr.h>

#define __init __section(".init.text")
#define __exit __section(".exit.text")

// ------------------------------------------------------------------------
#endif /* _CPU_INIT_H */
