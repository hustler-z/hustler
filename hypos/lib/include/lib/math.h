/**
 * Hustler's Project
 *
 * File:  math.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _LIB_MATH_H
#define _LIB_MATH_H
// ------------------------------------------------------------------------
#include <generic/type.h>

#define do_div(n,base) ({           \
    u32 __base = (base);            \
    u32 __rem;                      \
    __rem = ((u64)(n)) % __base;    \
    (n) = ((u64)(n)) / __base;      \
    __rem;                          \
})

#define max(x, y) ({\
    typeof(x) _max1 = (x);          \
    typeof(y) _max2 = (y);          \
    (void) (&_max1 == &_max2);      \
    _max1 > _max2 ? _max1 : _max2; })

#define min(x, y) ({                \
    typeof(x) _min1 = (x);          \
    typeof(y) _min2 = (y);          \
    (void) (&_min1 == &_min2);      \
    _min1 < _min2 ? _min1 : _min2; })

#define roundup(x, y) (					\
{							\
	const typeof(y) __y = y;			\
	(((x) + (__y - 1)) / __y) * __y;		\
}							\
)
#define rounddown(x, y) (				\
{							\
	typeof(x) __x = (x);				\
	__x - (__x % (y));				\
}							\
)

#define REPEAT_BYTE(x)	((~0ul / 0xff) * (x))

#define ALIGN(x,a)		__ALIGN_MASK((x),(typeof(x))(a)-1)
#define ALIGN_DOWN(x, a)	ALIGN((x) - ((a) - 1), (a))
#define __ALIGN_MASK(x,mask)	(((x)+(mask))&~(mask))
#define PTR_ALIGN(p, a)		((typeof(p))ALIGN((unsigned long)(p), (a)))
#define IS_ALIGNED(x, a)		(((x) & ((typeof(x))(a) - 1)) == 0)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

#define FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define DIV_ROUND_DOWN_ULL(ll, d) \
	({ unsigned long long _tmp = (ll); do_div(_tmp, d); _tmp; })

#define DIV_ROUND_UP_ULL(ll, d)		DIV_ROUND_DOWN_ULL((ll) + (d) - 1, (d))

#define ROUND(a, b)		(((a) + (b) - 1) & ~((b) - 1))

#define DIV_ROUND_CLOSEST(x, divisor) ({		\
    typeof(x) __x = x;				            \
    typeof(divisor) __d = divisor;			    \
    (((typeof(x))-1) > 0 ||				        \
     ((typeof(divisor))-1) > 0 || (__x) > 0) ?	\
        (((__x) + ((__d) / 2)) / (__d)) :	    \
        (((__x) - ((__d) / 2)) / (__d));	    \
})

#define DIV_ROUND_CLOSEST_ULL(x, divisor) ({	\
    typeof(divisor) __d = divisor;			    \
    unsigned long long _tmp = (x) + (__d) / 2;	\
    do_div(_tmp, __d);				            \
    _tmp;						                \
})

#define mult_frac(x, numer, denom) ({			\
    typeof(x) quot = (x) / (denom);			    \
    typeof(x) rem  = (x) % (denom);			    \
    (quot * (numer)) + ((rem * (numer)) / (denom));	\
})

#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))
#define lower_32_bits(n) ((u32)(n))

#define abs(x) ({						       \
    long ret;					               \
    if (sizeof(x) == sizeof(long)) {		   \
        long __x = (x);				           \
        ret = (__x < 0) ? -__x : __x;		   \
    } else {					               \
        int __x = (x);				           \
        ret = (__x < 0) ? -__x : __x;		   \
    }						                   \
    ret;						               \
})

#define abs64(x) ({				               \
    s64 __x = (x);			                   \
    (__x < 0) ? -__x : __x;		               \
})

#define __find_closest(x, a, as, op) ({			         \
    typeof(as) __fc_i, __fc_as = (as) - 1;				 \
    typeof(x) __fc_x = (x);						         \
    typeof(*a) const *__fc_a = (a);					     \
    for (__fc_i = 0; __fc_i < __fc_as; __fc_i++) {		 \
        if (__fc_x op DIV_ROUND_CLOSEST(__fc_a[__fc_i] + \
                        __fc_a[__fc_i + 1], 2))	         \
            break;						                 \
    }								                     \
    (__fc_i);							                 \
})

#define find_closest(x, a, as) __find_closest(x, a, as, <=)

unsigned int rand(void);
void srand(unsigned int seed);
// ------------------------------------------------------------------------
#endif /* _LIB_MATH_H */
