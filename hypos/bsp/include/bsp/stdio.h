/**
 * Hustler's Project
 *
 * File:  stdio.h
 * Date:  2024/06/05
 * Usage:
 */

#ifndef _BSP_STDIO_H
#define _BSP_STDIO_H
// --------------------------------------------------------------
#include <common/type.h>
#include <common/ccattr.h>
#include <lib/args.h>

/* stdin */
int getchar(void);
int tstc(void);

/* stdout */
void putc(const char c);
void puts(const char *s);
void flush(void);

void __pr(1, 2) pr(const char *fmt, ...);

#define HYPOS_PBSIZE        (1024)

void vpr_common(const char *fmt, va_list args);
int vspr(char *buf, const char *fmt, va_list args);
int snpr(char *buf, size_t size, const char *fmt, ...);
int vsnpr(char *buf, size_t size, const char *fmt,
        va_list args);
int vscnpr(char *buf, size_t size, const char *fmt,
        va_list args);
// --------------------------------------------------------------
#define stdin		0
#define stdout		1
#define stderr		2
#define MAX_FILES	3

/* stderr */
#define eputc(c)		        fputc(stderr, c)
#define eputs(s)		        fputs(stderr, s)
#define eflush()		        fflush(stderr)
#define eprintf(fmt, ...)	fprintf(stderr, fmt, ##__VA_ARGS__)

int __pr(2, 3) fpr(int file, const char *fmt, ...);
void fputs(int file, const char *s);
void fputc(int file, const char c);
void fflush(int file);
int  ftstc(int file);
int  fgetc(int file);
// --------------------------------------------------------------
#endif /* _BSP_STDIO_H */
