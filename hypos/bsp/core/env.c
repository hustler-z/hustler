/**
 * Hustler's Project
 *
 * File:  env.c
 * Date:  2024/06/05
 * Usage:
 */

#include <common/type.h>
// --------------------------------------------------------------
struct env_entry {
    char *name;
    char *value;
};

enum env_id {
    ENV_DEF = 0,
};

static struct env_entry envs[] = {
    {"name", "value"},
};

char *env_get(const char *name)
{
    /* TODO
     */
    return NULL;
}

int env_set(const char *name, const char *value)
{
    /* TODO
     */
    return 0;
}
// --------------------------------------------------------------
