#define _BSD_SOURCE
#ifdef _GNU_SOURCE
#define HAD_SOURCE
#undef _GNU_SOURCE
#endif
#include <fcntl.h>
#ifdef HAD_SOURCE
#undef HAD_SOURCE
#define _GNU_SOURCE
#endif

int creat(const char *filename, mode_t mode)
{
	return open(filename, O_CREAT|O_WRONLY|O_TRUNC, mode);
}

weak_alias(creat, creat64);
