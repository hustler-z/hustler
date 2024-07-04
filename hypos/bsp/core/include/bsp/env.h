/**
 * Hustler's Project
 *
 * File:  env.h
 * Date:  2024/06/05
 * Usage:
 */

#ifndef _BSP_ENV_H
#define _BSP_ENV_H
// --------------------------------------------------------------

char *env_get(const char *name);
int env_set(const char *name, const char *value);

// --------------------------------------------------------------
#endif /* _BSP_ENV_H */
