/*
 * Copyright (c) 2017, Linaro Limited. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	uniproton/io.h
 * @brief	Uniproton specific io definitions.
 */

#ifndef __METAL_IO__H__
#error "Include metal/io.h instead of metal/uniproton/io.h"
#endif

#ifndef __METAL_UNIPROTON_IO__H__
#define __METAL_UNIPROTON_IO__H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef METAL_INTERNAL
/**
 * @brief memory mapping for an I/O region
 */
static inline void metal_sys_io_mem_map(struct metal_io_region *io)
{
	(void)io;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* __METAL_UNIPROTON_IO__H__ */
