/*
 * Copyright (c) 2017, Linaro Limited. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	uniproton/mutex.h
 * @brief	Uniproton mutex primitives for libmetal.
 */

#ifndef __METAL_MUTEX__H__
#error "Include metal/mutex.h instead of metal/uniproton/mutex.h"
#endif

#ifndef __METAL_UNIPROTON_MUTEX__H__
#define __METAL_UNIPROTON_MUTEX__H__

#include "errno.h"
#include "pthread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef pthread_mutex_t metal_mutex_t;

/*
 * METAL_MUTEX_INIT - used for initializing an mutex element in a static struct
 * or global
 */
#define METAL_MUTEX_INIT(m) MUTEX_INITIALIZER
/*
 * METAL_MUTEX_DEFINE - used for defining and initializing a global or
 * static singleton mutex
 */
#define METAL_MUTEX_DEFINE(m) metal_mutex_t m = MUTEX_INITIALIZER

static inline void __metal_mutex_init(metal_mutex_t *mutex)
{
	pthread_mutex_init(mutex, NULL);
}

static inline void __metal_mutex_deinit(metal_mutex_t *mutex)
{
	pthread_mutex_destroy(mutex);
}

static inline int __metal_mutex_try_acquire(metal_mutex_t *mutex)
{
	return pthread_mutex_trylock(mutex);
}

static inline int __metal_mutex_is_acquired(metal_mutex_t *mutex)
{
	
}

static inline void __metal_mutex_acquire(metal_mutex_t *mutex)
{
	pthread_mutex_lock(mutex);
}

static inline void __metal_mutex_release(metal_mutex_t *mutex)
{
	pthread_mutex_unlock(mutex);
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_UNIPROTON_MUTEX__H__ */
