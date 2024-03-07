/*
 * Copyright (c) 2018, Linaro Limited. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	uniproton/sleep.h
 * @brief	Uniproton sleep primitives for libmetal.
 */

#ifndef __METAL_SLEEP__H__
#error "Include metal/sleep.h instead of metal/uniproton/sleep.h"
#endif

#ifndef __METAL_UNIPROTON_SLEEP__H__
#define __METAL_UNIPROTON_SLEEP__H__

#include "prt_task.h"
#include "prt_config.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline int __metal_sleep_usec(unsigned int usec)
{
	return PRT_TaskDelay((U32)(OS_TICK_PER_SECOND * usec / 1000000));
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_UNIPROTON_SLEEP__H__ */
