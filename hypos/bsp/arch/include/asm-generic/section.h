/**
 * Hustler's Project
 *
 * File:  section.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _GENERIC_SECTION_H
#define _GENERIC_SECTION_H
// --------------------------------------------------------------
#include <generic/ccattr.h>

#define __initdata  __section(".data.init")
#define __bootfunc  __section(".boot.setup")

// --------------------------------------------------------------
#endif /* _GENERIC_SECTION_H */
