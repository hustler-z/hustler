/*
 * Copyright (c) 2018, Linaro Limited. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	uniproton/cache.h
 * @brief	Uniproton cache operation primitives for libmetal.
 */

#ifndef __METAL_CACHE__H__
#error "Include metal/cache.h instead of metal/uniproton/cache.h"
#endif

#ifndef __METAL_UNIPROTON_CACHE__H__
#define __METAL_UNIPROTON_CACHE__H__

#ifdef __cplusplus
extern "C" {
#endif

static inline void __metal_cache_flush(void *addr, unsigned int len)
{
	(void)addr;
	(void)len;
}

static inline void __metal_cache_invalidate(void *addr, unsigned int len)
{
	(void)addr;
	(void)len;
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_UNIPROTON_CACHE__H__ */
