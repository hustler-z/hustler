/**
 * Hustler's Project
 *
 * File:  command.h
 * Date:  2024/05/20
 * Usage: general commands interact with console
 */

#ifndef _BSP_COMMAND_H
#define _BSP_COMMAND_H
// --------------------------------------------------------------

void __reset_cmd_timeout(void);
int __tstc_timeout(void);

// --------------------------------------------------------------
enum command_ret_t {
	CMD_RET_SUCCESS,	/* 0 = Success */
	CMD_RET_FAILURE,	/* 1 = Failure */
	CMD_RET_USAGE = -1,	/* Failure, please report 'usage' error */
};

#define CMD_DATA_SIZE
#define CMD_DATA_SIZE_ERR	(-1)
#define CMD_DATA_SIZE_STR	(-2)

/* Command Table
 */
struct cmd_tbl {
    char *name;		/* Command Name			*/
    int  maxargs;	/* maximum number of arguments	*/
                    /*
                     * Same as ->cmd() except the command
                     * tells us if it can be repeated.
                     * Replaces the old ->repeatable field
                     * which was not able to make
                     * repeatable property different for
                     * the main command and sub-commands.
                     */
    int (*cmd_rep)(struct cmd_tbl *cmd, int flags, int argc,
                   char *const argv[], int *repeatable);
                    /* Implementation function	*/
    int (*cmd)(struct cmd_tbl *cmd, int flags, int argc,
                   char *const argv[]);
    char *usage;    /* Usage message	(short)	*/
};

int cmd_always_repeatable(struct cmd_tbl *cmdtp, int flag, int argc,
                char *const argv[], int *repeatable);
int cmd_never_repeatable(struct cmd_tbl *cmdtp, int flag, int argc,
                char *const argv[], int *repeatable);

#define _CMD_HELP(x)
#define _CMD_COMPLETE(x)

#define HYPOS_CMD_MKENT(_name, _maxargs, _rep, _cmd,	        \
                _usage, _help, _comp)			                \
        { #_name, _maxargs,					                    \
         _rep ? cmd_always_repeatable : cmd_never_repeatable,	\
         _cmd, _usage, _CMD_HELP(_help) _CMD_COMPLETE(_comp) }

#define HYPOS_CMD(_name, _maxargs, _rep, _cmd, _usage, _help)   \
    entry_declare(struct cmd_tbl, _name, cmd) =                 \
        HYPOS_CMD_MKENT(_name, _maxargs, _rep, _cmd,	        \
						_usage, _help, NULL);

enum command_ret_t cmd_process(int flag, int argc, char *const argv[],
			       int *repeatable, unsigned long *ticks);
struct cmd_tbl *find_cmd(const char *cmd);
struct cmd_tbl *find_cmd_tbl(const char *cmd, struct cmd_tbl *table,
			     int table_len);

// --------------------------------------------------------------
#endif /* _BSP_COMMAND_H */
