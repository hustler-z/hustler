/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* This is a sample demonstration application that showcases usage of remoteproc
and rpmsg APIs on the remote core. This application is meant to run on the remote CPU 
running baremetal code. This application receives two matrices from the master, 
multiplies them and returns the result to the master core. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <openamp/open_amp.h>
#include "platform_info.h"
#include "rpmsg_service.h"
#include "fdebug.h"

#define     MAT_MULT_MASTER_DEBUG_TAG "    MASTER_03"
#define     MAT_MULT_MASTER_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( MAT_MULT_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)
#define     MAT_MULT_MASTER_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( MAT_MULT_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)
#define     MAT_MULT_MASTER_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( MAT_MULT_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)

#define	MAX_SIZE      6
#define NUM_MATRIX    2

typedef struct _matrix {
	unsigned int size;
	unsigned int elements[MAX_SIZE][MAX_SIZE];
} matrix;

/* Globals */
static struct rpmsg_endpoint lept;
static struct _matrix i_matrix[2];
static struct _matrix e_matrix;
static unsigned int result_returned = 0;
static int err_cnt = 0;
static int ept_deleted = 0;

/**
 * _gettimeofday() is called from time() which is used by srand() to generate
 * random number. It is defined here in case this function is not defined in
 * library.
 */
int __attribute__((weak)) _gettimeofday(struct timeval *tv, void *tz)
{
	(void)tv;
	(void)tz;
	return 0;
}

static void matrix_print(struct _matrix *m)
{
	unsigned int i, j;

	/* Generate two random matrices */
	MAT_MULT_MASTER_DEBUG_I("Printing matrix... \r\n");

	for (i = 0; i < m->size; ++i) {
		for (j = 0; j < m->size; ++j)
			printf(" %u ", m->elements[i][j]);
		printf("\r\n");
	}
}

static void generate_matrices(int num_matrices,
			      unsigned int matrix_size, void *p_data)
{
	unsigned int i, j, k;
	struct _matrix *p_matrix = p_data;
	unsigned long value;


	for (i = 0; i < (unsigned int)num_matrices; i++) {
		/* Initialize workload */
		p_matrix[i].size = matrix_size;

		MAT_MULT_MASTER_DEBUG_I("Input matrix %d \r\n", i);
		for (j = 0; j < matrix_size; j++) {
			printf("\r\n");
			for (k = 0; k < matrix_size; k++) {

				value = (rand() & 0x7F);
				value = value % 10;
				p_matrix[i].elements[j][k] = value;
				printf(" %u ", p_matrix[i].elements[j][k]);
			}
		}
		printf("\r\n");
	}

}

static void matrix_multiply_func(const matrix * m, const matrix * n, matrix * r)
{
	unsigned int i, j, k;

	memset(r, 0x0, sizeof(matrix));
	r->size = m->size;

	for (i = 0; i < m->size; ++i) {
		for (j = 0; j < n->size; ++j) {
			for (k = 0; k < r->size; ++k) {
				r->elements[i][j] += m->elements[i][k] * n->elements[k][j];
			}
		}
	}
}

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data,
			     size_t len, uint32_t src, void *priv)
{
	struct _matrix *r_matrix = (struct _matrix *)data;
	int i, j;

	(void)ept;
	(void)priv;
	(void)src;
	if (len != sizeof(struct _matrix)) {
		MAT_MULT_MASTER_DEBUG_E("Received matrix is of invalid len: %d:%lu\r\n",
			(int)sizeof(struct _matrix), (unsigned long)len);
		err_cnt++;
		return RPMSG_SUCCESS;
	}
	for (i = 0; i < MAX_SIZE; i++) {
		for (j = 0; j < MAX_SIZE; j++) {
			if (r_matrix->elements[i][j] !=
				e_matrix.elements[i][j]) {
				err_cnt++;
				break;
			}
		}
	}
	if (err_cnt) {
		MAT_MULT_MASTER_DEBUG_E("Result mismatched...\r\n");
		MAT_MULT_MASTER_DEBUG_E("Expected matrix:\r\n");
		matrix_print(&e_matrix);
		MAT_MULT_MASTER_DEBUG_E("Actual matrix:\r\n");
		matrix_print(r_matrix);
	} else {
		result_returned = 1;
	}
	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	rpmsg_destroy_ept(&lept);
	MAT_MULT_MASTER_DEBUG_I("echo test: service is destroyed\r\n");
	ept_deleted = 1;
}

static void rpmsg_name_service_bind_cb(struct rpmsg_device *rdev,
				       const char *name, uint32_t dest)
{
	if (strcmp(name, RPMSG_SERVICE_NAME))
		MAT_MULT_MASTER_DEBUG_E("Unexpected name service %s.\r\n", name);
	else
		(void)rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
				       RPMSG_ADDR_ANY, dest,
				       rpmsg_endpoint_cb,
				       rpmsg_service_unbind);

}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
static int app (struct rpmsg_device *rdev, void *priv)
{
	int c;
	int ret;

	ept_deleted = 0;

	MAT_MULT_MASTER_DEBUG_I("Compute thread unblocked ..\r\n");
	MAT_MULT_MASTER_DEBUG_I("It will generate two random matrices.\r\n");
	MAT_MULT_MASTER_DEBUG_I("Send to the remote and get the computation result back.\r\n");
	MAT_MULT_MASTER_DEBUG_I("It will then check if the result is expected.\r\n");

	/* Create RPMsg endpoint */
	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
			       RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			       rpmsg_endpoint_cb, rpmsg_service_unbind);
	if (ret) {
		MAT_MULT_MASTER_DEBUG_E("Failed to create RPMsg endpoint.\r\n");
		return ret;
	}

	while (!is_rpmsg_ept_ready(&lept))
		platform_poll(priv);

	MAT_MULT_MASTER_DEBUG_I("RPMSG endpoint is binded with remote.\r\n");
	err_cnt = 0;
	srand(time(NULL));
	for (c = 0; c < 200; c++) {
		generate_matrices(2, MAX_SIZE, i_matrix);
		matrix_multiply_func(&i_matrix[0], &i_matrix[1], &e_matrix);
		result_returned = 0;
		ret = rpmsg_send(&lept, i_matrix, sizeof(i_matrix));

		if (ret < 0) {
			MAT_MULT_MASTER_DEBUG_I("Error sending data...\r\n");
			break;
		}
		MAT_MULT_MASTER_DEBUG_I("Matrix multiply: sent : %lu\r\n",
			(unsigned long)sizeof(i_matrix));
		do {
			platform_poll(priv);
		} while (!result_returned && !err_cnt && !ept_deleted);

		if (err_cnt)
			break;
	}

	MAT_MULT_MASTER_DEBUG_I("**********************************\r\n");
	MAT_MULT_MASTER_DEBUG_I(" Test Results: Error count = %d \r\n", err_cnt);
	MAT_MULT_MASTER_DEBUG_I("**********************************\r\n");

	/* Detroy RPMsg endpoint */
	rpmsg_destroy_ept(&lept);
	MAT_MULT_MASTER_DEBUG_I("Quitting application .. Matrix multiplication end\r\n");

	return 0;
}

int matrix_multiply(struct rpmsg_device *rdev, void *priv)
{
	metal_assert(rdev);
    metal_assert(priv);
	int ret;

	MAT_MULT_MASTER_DEBUG_I("Starting matrix_multiply application...\r\n");
	ret = app(rdev, priv);
	if (ret != 0)
    {
        MAT_MULT_MASTER_DEBUG_E("Matrix_multiply application error,code:0x%x",ret);
        return ret;
    }

	MAT_MULT_MASTER_DEBUG_I("Stopping matrix_multiply application...\r\n");

	return ret;
}

