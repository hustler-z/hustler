/**
 * Hustler's Project
 *
 * File:  type.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _BSP_TYPE_H
#define _BSP_TYPE_H
// --------------------------------------------------------------

typedef char               s8;
typedef short              s16;
typedef int                s32;
typedef long               s64;

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long      u64; /* ARM64 case */

typedef unsigned long      size_t;
typedef long               ssize_t;
typedef unsigned long      uptr_t;
typedef long               ptrdiff_t;

typedef u16                __le16;
typedef u32                __le32;
typedef u64                __le64;
#define NULL               ((void *)0)

typedef unsigned long      hva_t;
typedef unsigned long      hpa_t;
typedef unsigned long      gva_t;
typedef unsigned long      gpa_t;

typedef s64                stime_t;

#define __I   volatile const
#define __O   volatile
#define __IO  volatile

#define __ro  const volatile
#define __rw  volatile
#define __wo  volatile

typedef u64   register_t;

typedef u32   bw_u32;
typedef u64   bw_u64;

typedef enum {
    false = 0,
    true,
} bool;

#define USHRT_MAX	((u16)(~0U))
#define SHRT_MAX	((s16)(USHRT_MAX>>1))
#define SHRT_MIN	((s16)(-SHRT_MAX - 1))
#define INT_MAX		((int)(~0U>>1))
#define INT_MIN		(-INT_MAX - 1)
#define UINT_MAX	(~0U)
#define LONG_MAX	((long)(~0UL>>1))
#define LONG_MIN	(-LONG_MAX - 1)
#define ULONG_MAX	(~0UL)
#define LLONG_MAX	((long long)(~0ULL>>1))
#define LLONG_MIN	(-LLONG_MAX - 1)
#define ULLONG_MAX	(~0ULL)
#define SIZE_MAX	(~(size_t)0)
#define SSIZE_MAX	((ssize_t)(SIZE_MAX >> 1))

#define U8_MAX		((u8)~0U)
#define S8_MAX		((s8)(U8_MAX>>1))
#define S8_MIN		((s8)(-S8_MAX - 1))
#define U16_MAX		((u16)~0U)
#define S16_MAX		((s16)(U16_MAX>>1))
#define S16_MIN		((s16)(-S16_MAX - 1))
#define U32_MAX		((u32)~0U)
#define S32_MAX		((s32)(U32_MAX>>1))
#define S32_MIN		((s32)(-S32_MAX - 1))
#define U64_MAX		((u64)~0ULL)
#define S64_MAX		((s64)(U64_MAX>>1))
#define S64_MIN		((s64)(-S64_MAX - 1))

#define UINT32_MAX	U32_MAX
#define UINT64_MAX	U64_MAX

#define INT32_MAX	S32_MAX

#define NULL_PTR(p) ((void *)(p) == NULL)

/* Address Pointers */
typedef unsigned long __attribute__((__mode__(__pointer__))) ap_t;

#define __void__(x)     ((void *)(unsigned long)(x))
// --------------------------------------------------------------
#include <org/bitops.h>

#define BITS_TO_LONGS(bits) \
    (((bits) + BITS_PER_LONG - 1) / BITS_PER_LONG)

#define DECLARE_BITMAP(name, bits) \
    unsigned long name[BITS_TO_LONGS(bits)]

#define typecheck(type, x) ({      \
    type __dummy;                  \
    typeof(x) __dummy2;            \
    (void)(&__dummy == &__dummy2); \
    1;                             \
})

#define typecheck_fn(type, function) ({ \
    typeof(type) __tmp = function;      \
    (void)__tmp;                        \
})

#define TYPE_SAFE(type, name)                       \
typedef struct { type name; } name##_t;             \
static inline name##_t name##_set(type n)           \
{ return (name##_t){ n }; }                         \
static inline type name##_get(name##_t n)           \
{ return n.name; }

// --------------------------------------------------------------
#endif /* _BSP_TYPE_H */
