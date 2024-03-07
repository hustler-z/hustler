/*
 * Copyright (c) 2017, Linaro Limited. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	uniproton/device.c
 * @brief	Uniproton libmetal device definitions.
 */

#include <metal/device.h>
#include <metal/io.h>
#include <metal/sys.h>
#include <metal/utilities.h>

int metal_generic_dev_sys_open(struct metal_device *dev)
{
	return 0;
}

