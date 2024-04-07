/**
 * ---------------------------------------------------------
 * Header file for string operations
 *
 *
 * Date: 2024/04/06 -
 * ---------------------------------------------------------
 **/

#include <stdint.h>

typedef uint64_t size_t;

char *strcpy(char *dest, const char *src);;
char *strncpy(char *dest, const char *src, size_t count);
size_t strlcpy(char *dest, const char *src, size_t size);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t count);
size_t strlcat(char *dest, const char *src, size_t count);
int strcmp(const char *cs, const char *ct);
int strncmp(const char *cs, const char *ct, size_t count);
char *strchr(const char *s, int c);
char *strchrnul(const char *s, int c);
char *strnchrnul(const char *s, size_t count, int c);
char *strrchr(const char *s, int c);
char *strnchr(const char *s, size_t count, int c);
char *skip_spaces(const char *str);
char *strim(char *s);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t count);
size_t strspn(const char *s, const char *accept);
size_t strcspn(const char *s, const char *reject);
char *strpbrk(const char *cs, const char *ct);
char *strsep(char **s, const char *ct);
int match_string(const char * const *array, size_t n, const char *string);
void *memset(void *s, int c, size_t count);
void *memset16(uint16_t *s, uint16_t v, size_t count);
void *memset32(uint32_t *s, uint32_t v, size_t count);
void *memset64(uint64_t *s, uint64_t v, size_t count);
void *memcpy(void *dest, const void *src, size_t count);
void *memmove(void *dest, const void *src, size_t count);
int memcmp(const void *cs, const void *ct, size_t count);
int bcmp(const void *a, const void *b, size_t len);
void *memscan(void *addr, int c, size_t size);
char *strstr(const char *s1, const char *s2);
char *strnstr(const char *s1, const char *s2, size_t len);
void *memchr(const void *s, int c, size_t n);
char *strreplace(char *s, char old, char new);
