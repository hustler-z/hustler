

/**
 * Hustler's Project
 *
 * File:  common_ccattr.h
 * Date:  2024/05/22
 * Usage: common compiler attributes defined in this header file
 */

#ifndef _COMMON_CCATTR_H
#define _COMMON_CCATTR_H
// ------------------------------------------------------------------------

#define __aligned(x)       __attribute__((__aligned__(x)))
#define __packed           __attribute__((__packed__))
#define __maybe_unused     __attribute__((__unused__))
#define __section(section) __attribute__((__section__(section)))
#define __iomem
#define __force

// ------------------------------------------------------------------------
#endif /* _COMMON_CCATTR_H */
