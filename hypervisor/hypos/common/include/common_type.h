/**
 * Hustler's Project
 *
 * File:  common_type.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _COMMON_TYPE_H
#define _COMMON_TYPE_H
// ------------------------------------------------------------------------


typedef char               s8;
typedef short              s16;
typedef int                s32;
typedef long               s64;

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long      u64; /* ARM64 case */

typedef unsigned long      size_t;

typedef u16                __le16;
typedef u32                __le32;
typedef u64                __le64;
#define NULL               ((void *)0)

// ------------------------------------------------------------------------
#endif /* _COMMON_TYPE_H */
