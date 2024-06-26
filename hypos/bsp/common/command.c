/**
 * Hustler's Project
 *
 * File:  command.c
 * Date:  2024/05/20
 * Usage: general commands interact with console
 */

#include <asm/linker.h>
#include <common/type.h>
#include <common/errno.h>
#include <common/timer.h>
#include <bsp/command.h>
#include <bsp/console.h>
#include <bsp/period.h>
#include <bsp/debug.h>
#include <lib/strops.h>

// --------------------------------------------------------------
static u64 endtime;
static int retrytime;

void __init_cmd_timeout(void)
{
    retrytime = 0;
}

void __reset_cmd_timeout(void)
{
    endtime = endtick(retrytime);
}

int __tstc_timeout(void)
{
    while (!tstc()) {	/* while no incoming data */
        if (retrytime >= 0 && get_ticks() > endtime)
            return -ETIMEDOUT;
        schedule();
    }

    return 0;
}
// --------------------------------------------------------------
int cmd_always_repeatable(struct cmd_tbl *cmdtp, int flag, int argc,
        char *const argv[], int *repeatable)
{
    *repeatable = 1;

    return cmdtp->cmd(cmdtp, flag, argc, argv);
}

int cmd_never_repeatable(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[], int *repeatable)
{
    *repeatable = 0;

    return cmdtp->cmd(cmdtp, flag, argc, argv);
}
// --------------------------------------------------------------
/* find command table entry for a command */
struct cmd_tbl *find_cmd_tbl(const char *cmd, struct cmd_tbl *table,
        int table_len)
{
    struct cmd_tbl *cmdtp;
    struct cmd_tbl *cmdtp_temp = table;	/* Init value */
    const char *p;
    int len;
    int n_found = 0;

    if (!cmd)
        return NULL;
    /*
     * Some commands allow length modifiers (like "cp.b");
     * compare command name only until first dot.
     */
    len = ((p = strchr(cmd, '.')) == NULL) ? strlen(cmd) : (p - cmd);

    for (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
        if (strncmp(cmd, cmdtp->name, len) == 0) {
            if (len == strlen(cmdtp->name))
                return cmdtp;	/* full match */

            cmdtp_temp = cmdtp;	/* abbreviated command ? */
            n_found++;
        }
    }
    if (n_found == 1) {			/* exactly one match */
        return cmdtp_temp;
    }

    return NULL;	/* not found or ambiguous command */
}

static int cmd_call(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[], int *repeatable)
{
    int result;

    result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
    if (result)
        DEBUG("Command failed, result=%d\n", result);
    return result;
}

struct cmd_tbl *find_cmd(const char *cmd)
{
    struct cmd_tbl *start = _entry_start(struct cmd_tbl, cmd);
    const int len = _entry_count(struct cmd_tbl, cmd);
    return find_cmd_tbl(cmd, start, len);
}

int cmd_usage(const struct cmd_tbl *cmdtp)
{
    DEBUG("%s - %s\n\n", cmdtp->name, cmdtp->usage);
    return 1;
}

enum command_ret_t cmd_process(int flag, int argc, char *const argv[],
			       int *repeatable, unsigned long *ticks)
{
    enum command_ret_t rc = CMD_RET_SUCCESS;
    struct cmd_tbl *cmdtp;

    /* Look up command in command table */
    cmdtp = find_cmd(argv[0]);
    if (cmdtp == NULL) {
        DEBUG("Unknown command '%s' - try 'help'\n", argv[0]);
        return 1;
    }

    /* found - check max args */
    if (argc > cmdtp->maxargs)
        rc = CMD_RET_USAGE;

    /* If OK so far, then do the command */
    if (!rc) {
        int newrep;

        if (ticks)
            *ticks = get_timer(0);
        rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
        if (ticks)
            *ticks = get_timer(*ticks);
        *repeatable &= newrep;
    }
    if (rc == CMD_RET_USAGE)
        rc = cmd_usage(cmdtp);
    return rc;
}

// --------------------------------------------------------------

/* TODO COMMAND IMPLEMENTATION
 */
