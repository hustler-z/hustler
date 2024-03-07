#ifndef __FSL_WIFI_PORT_H__
#define __FSL_WIFI_PORT_H__

#include <string.h>
#include "ftypes.h"
#include "fdebug.h"
#include "fearly_uart.h"

#define PRINTF  printf
#define GETCHAR GetByte
#define PUTCHAR OutByte

#ifndef bool
#define bool  boolean
#endif

#ifndef true
#define true  1
#endif

#ifndef false
#define false 0
#endif

#endif