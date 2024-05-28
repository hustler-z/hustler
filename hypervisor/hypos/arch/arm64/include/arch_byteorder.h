/**
 * Hustler's Project
 *
 * File:  arch_byteorder.h
 * Date:  2024/05/20
 * Usage:
 */

#ifndef _ARCH_BYTEORDER_H
#define _ARCH_BYTEORDER_H

#include <common_ccattr.h>

#define cpu_to_le16  ((__force __le16)((u16)(x)))
#define cpu_to_le32  ((__force __le32)((u32)(x)))
#define cpu_to_le64  ((__force __le64)((u64)(x)))

#endif /* _ARCH_BYTEORDER_H */
