/**
 * Hustler's Project
 *
 * File:  strops.h
 * Date:  2024/05/22
 * Usage:
 */

#ifndef _LIB_STROPS_H
#define _LIB_STROPS_H
// ------------------------------------------------------------------------
#include <bsp/type.h>

char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t count);
size_t strlcpy(char *dest, const char *src, size_t size);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t count);
size_t strlcat(char *dest, const char *src, size_t count);
int strcmp(const char *cs, const char *ct);
int strncmp(const char *cs, const char *ct, size_t count);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strnchr(const char *s, size_t count, int c);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t count);
void *memset(void *s, int c, size_t count);
void *memcpy(void *dest, const void *src, size_t count);
void *memmove(void *dest, const void *src, size_t count);
int memcmp(const void *cs, const void *ct, size_t count);
void *memscan(void *addr, int c, size_t size);
char *strstr(const char *s1, const char *s2);
char *strnstr(const char *s1, const char *s2, size_t len);
void *memchr(const void *s, int c, size_t n);
char *strdup(const char *s);
char *strndup(const char *s, size_t n);

// ------------------------------------------------------------------------
#endif /* _LIB_STROPS_H */
