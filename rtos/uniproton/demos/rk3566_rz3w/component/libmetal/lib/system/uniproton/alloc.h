/*
 * Copyright (c) 2017, Linaro Limited. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	uniproton/alloc.h
 * @brief	Uniproton libmetal memory allocattion definitions.
 */

#ifndef __METAL_ALLOC__H__
#error "Include metal/alloc.h instead of metal/uniproton/alloc.h"
#endif

#ifndef __METAL_UNIPROTON_ALLOC__H__
#define __METAL_UNIPROTON_ALLOC__H__

#include "prt_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void *metal_allocate_memory(unsigned int size)
{
	return PRT_MemAlloc(OS_MID_SYS, OS_MEM_DEFAULT_PT0, size);
}

static inline void metal_free_memory(void *ptr)
{
	PRT_MemFree(OS_MID_SYS, ptr);
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_UNIPROTON_ALLOC__H__ */
