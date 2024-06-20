/**
 * Hustler's Project
 *
 * File:  console.c
 * Date:  2024/05/20
 * Usage: console initialization
 */

#include <asm/debug.h>
#include <asm-generic/globl.h>
#include <asm-generic/section.h>
#include <bsp/console.h>
#include <bsp/command.h>
#include <bsp/debug.h>
#include <bsp/env.h>
#include <bsp/serial.h>
#include <bsp/alloc.h>
#include <bsp/period.h>
#include <bsp/sdev.h>
#include <generic/ccattr.h>
#include <generic/mmap.h>
#include <generic/errno.h>
#include <generic/exit.h>
#include <generic/timer.h>
#include <lib/strops.h>
#include <lib/ctype.h>
#include <lib/math.h>

#include <hyp/vm.h>

/* TODO Needs to modify
 */
#define SYS_CBSIZE               1024
#define SYS_PROMPT               "(hypos) "
#define SYS_MAXARGS              6
#define SYS_PBSIZE               HYPOS_PBSIZE

#define CMD_FLAG_REPEAT          0x0001
#define CMD_FLAG_ENV             0x0002
#define DEBUG_PARSER	         0

extern struct stdio_dev *stdio_devices[];

// --------------------------------------------------------------

int serial_pr(const char *fmt, ...)
{
    va_list args;
    unsigned int i;
    char prbuf[SYS_PBSIZE];

    va_start(args, fmt);

    /* For this to work, prbuf must be larger than
     * anything we ever want to print.
     */
    i = vscnpr(prbuf, sizeof(prbuf), fmt, args);
    va_end(args);

    serial_puts(prbuf);
    return i;
}

int getc(void)
{
    if (get_globl()->smode == GLB_STDIO_SERIAL)
        return fgetc(stdin);
    else
        return serial_getc();
}

int tstc(void)
{
    if (get_globl()->smode == GLB_STDIO_SERIAL)
        return ftstc(stdin);
    else
        return serial_tstc();
}

#define PRE_CON_BUF_ADDR 0x0
#define PRE_CON_BUF_SIZE 1024

#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)PRE_CON_BUF_SIZE)

static void pre_console_putc(const char c)
{
    char *buffer;

    buffer = map_hypmem(PRE_CON_BUF_ADDR, PRE_CON_BUF_SIZE);

    buffer[CIRC_BUF_IDX(get_globl()->precon_buf_idx++)] = c;

    unmap_hypmem(buffer);
}

static void pre_console_puts(const char *s)
{
    while (*s)
        pre_console_putc(*s++);
}

void putc(const char c)
{
    if (get_globl()->smode == GLB_STDIO_SERIAL)
        fputc(stdout, c);
    else if (get_globl()->smode == GLB_BASIC_SERIAL) {
        pre_console_putc(c);
        serial_putc(c);
    } else
        early_putc(c);
}

void puts(const char *s)
{
    if (get_globl()->smode == GLB_STDIO_SERIAL)
        fputs(stdout, s);
    else if (get_globl()->smode == GLB_BASIC_SERIAL) {
        pre_console_puts(s);
        serial_puts(s);
    } else
        early_debug(s);
}

void flush(void)
{
    if (get_globl()->smode == GLB_STDIO_SERIAL)
        fflush(stdout);
    else if (get_globl()->smode == GLB_BASIC_SERIAL)
        serial_flush();
    else
        early_flush();
}
// --------------------------------------------------------------

/* test if ctrl-c was pressed */
static int ctrlc_disabled = 0;
static int ctrlc_was_pressed = 0;

int ctrlc(void)
{
    if (!ctrlc_disabled) {
        if (tstc()) {
            switch (getc()) {
            case 0x03:		/* ^C - Control C */
                ctrlc_was_pressed = 1;
                return 1;
            default:
                break;
            }
        }
    }

    return 0;
}

int disable_ctrlc(int disable)
{
    int prev = ctrlc_disabled;	/* save previous state */

    ctrlc_disabled = disable;
    return prev;
}

int had_ctrlc (void)
{
    return ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
    ctrlc_was_pressed = 0;
}

/* STDIO CONSOLE IMPLEMENTATION based on U-Boot
 *
 */

// --------------------------------------------------------------
static struct stdio_dev *tstcdev;
struct stdio_dev **console_devices[MAX_FILES];
int cd_count[MAX_FILES];

#define for_each_console_dev(i, file, dev)				         \
    for (i = 0;							                         \
         i < cd_count[file] && (dev = console_devices[file][i]); \
         i++)

static int console_getc(int file)
{
    unsigned char ret;

    /* This is never called with testcdev == NULL */
    ret = tstcdev->getc(tstcdev);
    tstcdev = NULL;
    return ret;
}

static bool console_has_tstc(void)
{
    return !!tstcdev;
}

static int console_tstc(int file)
{
    int i, ret;
    struct stdio_dev *dev;
    int prev;

    prev = disable_ctrlc(1);
    for_each_console_dev(i, file, dev) {
        if (dev->tstc != NULL) {
            ret = dev->tstc(dev);
            if (ret > 0) {
                tstcdev = dev;
                disable_ctrlc(prev);
                return ret;
            }
        }
    }
    disable_ctrlc(prev);

    return 0;
}

static void console_putc(int file, const char c)
{
    int i;
    struct stdio_dev *dev;

    for_each_console_dev(i, file, dev) {
        if (dev->putc != NULL)
            dev->putc(dev, c);
    }
}

static bool console_dev_is_serial(struct stdio_dev *sdev)
{
    bool is_serial;

    if (sdev->flags & DEV_FLAGS_SERIAL) {
        struct hypos_device *dev = sdev->priv;

        is_serial = dev_get_type(dev) == HYP_DT_SERIAL;
    } else {
        is_serial = !strcmp(sdev->name, "serial");
    }

    return is_serial;
}

static void console_puts_select(int file, bool serial_only, const char *s)
{
    int i;
    struct stdio_dev *dev;

    for_each_console_dev(i, file, dev) {
        bool is_serial = console_dev_is_serial(dev);

        if (dev->puts && serial_only == is_serial)
            dev->puts(dev, s);
    }
}

void console_puts_select_stderr(bool serial_only, const char *s)
{
    if (get_globl()->flags & GLB_DEVICE_INIT)
        console_puts_select(stderr, serial_only, s);
}

static void console_puts(int file, const char *s)
{
    int i;
    struct stdio_dev *dev;

    for_each_console_dev(i, file, dev) {
        if (dev->puts != NULL)
            dev->puts(dev, s);
    }
}

static void console_flush(int file)
{
    int i;
    struct stdio_dev *dev;

    for_each_console_dev(i, file, dev) {
        if (dev->flush != NULL)
            dev->flush(dev);
    }
}
// --------------------------------------------------------------
#define CONSOLE_MUX_ENABLE        (1)
#define WATCHDOG_ENABLE           (1)

int fgetc(int file)
{
    if ((unsigned int)file < MAX_FILES) {
        /*
         * Effectively poll for input wherever it may be available.
         */
        for (;;) {
            schedule();
            if (CONSOLE_MUX_ENABLE) {
                /*
                 * Upper layer may have already called tstc() so
                 * check for that first.
                 */
                if (console_has_tstc())
                    return console_getc(file);
                console_tstc(file);
            } else {
                if (console_tstc(file))
                    return console_getc(file);
            }

            /*
             * If the watchdog must be rate-limited then it should
             * already be handled in board-specific code.
             */
            if (WATCHDOG_ENABLE)
                udelay(1);
        }
    }

    return -1;
}

int ftstc(int file)
{
    if ((unsigned int)file < MAX_FILES)
        return console_tstc(file);

    return -1;
}

void fputc(int file, const char c)
{
    if ((unsigned int)file < MAX_FILES)
        console_putc(file, c);
}

void fputs(int file, const char *s)
{
    if ((unsigned int)file < MAX_FILES)
        console_puts(file, s);
}

void fflush(int file)
{
    if ((unsigned int)file < MAX_FILES)
        console_flush(file);
}

int fprintf(int file, const char *fmt, ...)
{
    va_list args;
    unsigned int i;
    char prbuf[SYS_PBSIZE];

    va_start(args, fmt);

    /* For this to work, prbuf must be larger than
     * anything we ever want to print.
     */
    i = vscnpr(prbuf, sizeof(prbuf), fmt, args);
    va_end(args);

    /* Send to desired file */
    fputs(file, prbuf);
    return i;
}

// --------------------------------------------------------------
static int match_device(struct stdio_dev **set, const int n,
        struct stdio_dev *sdev)
{
    int i;

    for (i = 0; i < n; i++)
        if (sdev == set[i])
            return i;
    return -ENOENT;
}

static bool console_needs_start_stop(int file, struct stdio_dev *sdev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(cd_count); i++) {
        if (i == file)
            continue;

        if (match_device(console_devices[i], cd_count[i], sdev) >= 0)
            return false;
    }
    return true;
}

int console_start(int file, struct stdio_dev *sdev)
{
    int error;

    if (!console_needs_start_stop(file, sdev))
        return 0;

    /* Start new hypos_device */
    if (sdev->start) {
        error = sdev->start(sdev);
        /* If it's not started don't use it */
        if (error < 0)
            return error;
    }
    return 0;
}

void console_stop(int file, struct stdio_dev *sdev)
{
    if (!console_needs_start_stop(file, sdev))
        return;

    if (sdev->stop)
        sdev->stop(sdev);
}

static int console_setfile(int file, struct stdio_dev * dev)
{
    int error = 0;

    if (dev == NULL)
        return -1;

    switch (file) {
    case stdin:
    case stdout:
    case stderr:
        error = console_start(file, dev);
        if (error)
            break;

        /* Assign the new hypos_device (leaving the existing one started) */
        stdio_devices[file] = dev;

        /*
         * Update monitor functions
         * (to use the console stuff by other applications)
         */
        switch (file) {
        case stdin:
            get_globl()->fjmp->getc = getc;
            get_globl()->fjmp->tstc = tstc;
            break;
        case stdout:
            get_globl()->fjmp->putc  = putc;
            get_globl()->fjmp->puts  = puts;
            STDIO_DEV_ASSIGN_FLUSH(get_globl()->fjmp, flush);
            get_globl()->fjmp->pr = pr;
            break;
        }
        break;

    default:		/* Invalid file ID */
        error = -1;
    }
    return error;
}

struct stdio_dev *console_search_dev(int flags, const char *name)
{
    struct stdio_dev *dev;

    dev = stdio_get_by_name(name);

    if (dev && (dev->flags & flags))
        return dev;

    return NULL;
}

int console_assign(int file, const char *devname)
{
    int flag;
    struct stdio_dev *dev;

    flag = stdio_file_to_flags(file);
    if (flag < 0)
        return flag;

    dev = console_search_dev(flag, devname);

    if (dev)
        return console_setfile(file, dev);

    return -1;
}

// --------------------------------------------------------------

static const char erase_seq[] = "\b \b";	/* erase sequence */
static const char   tab_seq[] = "        ";	/* used to expand TABs */

char console_buffer[SYS_CBSIZE + 1];	    /* console I/O buffer	*/

static char *delete_char(char *buffer, char *p, int *colp,
        int *np, int plen)
{
    char *s;

    if (*np == 0)
        return p;

    if (*(--p) == '\t') {
        while (*colp > plen) {
            puts(erase_seq);
            (*colp)--;
        }
        for (s = buffer; s < p; ++s) {
            if (*s == '\t') {
                puts(tab_seq + ((*colp) & 07));
                *colp += 8 - ((*colp) & 07);
            } else {
                ++(*colp);
                putc(*s);
            }
        }
    } else {
        puts(erase_seq);
        (*colp)--;
    }
    (*np)--;

    return p;
}

int console_readline_into_buffer(const char *const prompt, char *buffer,
			     int timeout)
{
    char *p = buffer;
    char *p_buf = p;
    int	n = 0;				/* buffer index	*/
    int	plen = 0;			/* prompt length */
    int	col;				/* output column cnt */
    char	c;

    /* print prompt */
    if (prompt) {
        plen = strlen(prompt);
        puts(prompt);
    }
    col = plen;

    for (;;) {
        if (__tstc_timeout())
            return -2;	/* timed out */
        schedule();

        c = getc();

        /*
         * Special character handling
         */
        switch (c) {
        case '\r':			    /* Enter		*/
        case '\n':
            *p = '\0';
            puts("\r\n");
            return p - p_buf;

        case '\0':			    /* nul			*/
            continue;

        case 0x03:			    /* ^C - break	 */
            p_buf[0] = '\0';	/* discard input */
            return -1;

        case 0x15:			    /* ^U - erase line	*/
            while (col > plen) {
                puts(erase_seq);
                --col;
            }
            p = p_buf;
            n = 0;
            continue;

        case 0x17:			    /* ^W - erase word	*/
            p = delete_char(p_buf, p, &col, &n, plen);
            while ((n > 0) && (*p != ' '))
                p = delete_char(p_buf, p, &col, &n, plen);
            continue;

        case 0x08:			   /* ^H  - backspace	*/
        case 0x7F:			   /* DEL - backspace	*/
            p = delete_char(p_buf, p, &col, &n, plen);
            continue;

        default:
            /*
             * Must be a normal character then
             */
            if (n < SYS_CBSIZE-2) {
                if (c == '\t') {	/* expand TABs */
                    puts(tab_seq + (col & 07));
                    col += 8 - (col & 07);
                } else {
                    char __maybe_unused buf[2];
                    ++col;
                    buf[0] = c;
                    buf[1] = '\0';
                    puts(buf);
                }
                *p++ = c;
                ++n;
            } else {			/* Buffer full */
                putc('\a');
            }
        }
    }
}

int console_readline(const char *const prompt)
{
    /*
     * If console_buffer isn't 0-length the user will be prompted to modify
     * it instead of entering it from scratch as desired.
     */
    console_buffer[0] = '\0';

    return console_readline_into_buffer(prompt, console_buffer, 0);
}

int console_parse_line(char *line, char *argv[])
{
    int nargs = 0;

    while (nargs < SYS_MAXARGS) {
        /* skip any white space */
        while (isblank(*line))
            ++line;

        if (*line == '\0') {	/* end of line, no more args	*/
            argv[nargs] = NULL;

            return nargs;
        }

        argv[nargs++] = line;	/* begin of argument string	*/

        /* find end of string */
        while (*line && !isblank(*line))
            ++line;

        if (*line == '\0') {	/* end of line, no more args	*/
            argv[nargs] = NULL;

            return nargs;
        }

        *line++ = '\0';		/* terminate current arg	 */
    }

    DEBUG("** Too many args (max. %d) **\n", SYS_MAXARGS);

    return nargs;
}

int console_process_macros(const char *input, char *output, int max_size)
{
    char c, prev;
    const char *varname_start = NULL;
    int inputcnt = strlen(input);
    int outputcnt = max_size;
    int state = 0;		/* 0 = waiting for '$'  */
    int ret;

    /* 1 = waiting for '(' or '{' */
    /* 2 = waiting for ')' or '}' */
    /* 3 = waiting for '''  */
    char __maybe_unused *output_start = output;

    prev = '\0';		/* previous character   */

    while (inputcnt && outputcnt) {
        c = *input++;
        inputcnt--;

        if (state != 3) {
            /* remove one level of escape characters */
            if ((c == '\\') && (prev != '\\')) {
                if (inputcnt-- == 0)
                    break;
                prev = c;
                c = *input++;
            }
        }

        switch (state) {
        case 0:	/* Waiting for (unescaped) $    */
            if ((c == '\'') && (prev != '\\')) {
                state = 3;
                break;
            }
            if ((c == '$') && (prev != '\\')) {
                state++;
            } else {
                *(output++) = c;
                outputcnt--;
            }
            break;
        case 1:	/* Waiting for (        */
            if (c == '(' || c == '{') {
                state++;
                varname_start = input;
            } else {
                state = 0;
                *(output++) = '$';
                outputcnt--;

                if (outputcnt) {
                    *(output++) = c;
                    outputcnt--;
                }
            }
            break;
        case 2:	/* Waiting for )        */
            if (c == ')' || c == '}') {
                int i;
                char envname[SYS_CBSIZE], *envval;
                /* Varname # of chars */
                int envcnt = input - varname_start - 1;

                /* Get the varname */
                for (i = 0; i < envcnt; i++)
                    envname[i] = varname_start[i];
                envname[i] = 0;

                /* Get its value */
                envval = env_get(envname);

                /* Copy into the line if it exists */
                if (envval != NULL)
                    while ((*envval) && outputcnt) {
                        *(output++) = *(envval++);
                        outputcnt--;
                    }
                /* Look for another '$' */
                state = 0;
            }
            break;
        case 3:	/* Waiting for '        */
            if ((c == '\'') && (prev != '\\')) {
                state = 0;
            } else {
                *(output++) = c;
                outputcnt--;
            }
            break;
        }
        prev = c;
    }

    ret = inputcnt ? -ENOSPC : 0;
    if (outputcnt) {
        *output = 0;
    } else {
        *(output - 1) = 0;
        ret = -ENOSPC;
    }

    return ret;
}

int console_run_command(const char *cmd, int flag)
{
    char cmdbuf[SYS_CBSIZE];	  /* working copy of cmd		*/
    char *token;			      /* start of token in cmdbuf	*/
    char *sep;			          /* end of token (separator) in cmdbuf */
    char finaltoken[SYS_CBSIZE];
    char *str = cmdbuf;
    char *argv[SYS_MAXARGS + 1];  /* NULL terminated	*/
    int argc, inquotes;
    int repeatable = 1;
    int rc = 0;

    if (DEBUG_PARSER) {
        /* use puts - string may be loooong */
        puts(cmd ? cmd : "NULL");
        puts("\"\n");
    }
    clear_ctrlc();		/* forget any previous Control C */

    if (!cmd || !*cmd)
        return -1;	/* empty command */

    if (strlen(cmd) >= SYS_CBSIZE) {
        puts("## Command too long!\n");
        return -1;
    }

    strcpy(cmdbuf, cmd);

    /* Process separators and check for invalid
     * repeatable commands
     */
    while (*str) {
        /*
         * Find separator, or string end
         * Allow simple escape of ';' by writing "\;"
         */
        for (inquotes = 0, sep = str; *sep; sep++) {
            if ((*sep == '\'') &&
                (*(sep - 1) != '\\'))
                inquotes = !inquotes;

            if (!inquotes &&
                (*sep == ';') &&	    /* separator		 */
                (sep != str) &&	        /* past string start */
                (*(sep - 1) != '\\'))	/* and NOT escaped   */
                break;
        }

        /*
         * Limit the token to data between separators
         */
        token = str;
        if (*sep) {
            str = sep + 1;	/* start of command for next pass */
            *sep = '\0';
        } else {
            str = sep;	/* no more commands for next pass */
        }

        /* find macros in this token and replace them */
        console_process_macros(token, finaltoken,
                      sizeof(finaltoken));

        /* Extract arguments */
        argc = console_parse_line(finaltoken, argv);
        if (argc == 0) {
            rc = -1;	/* no command at all */
            continue;
        }

        if (cmd_process(flag, argc, argv, &repeatable, NULL))
            rc = -1;

        /* Did the user stop this? */
        if (had_ctrlc())
            return -1;	/* if stopped then not repeatable */
    }

    return rc ? rc : repeatable;
}

int console_run_command_list(char *cmd, int flag)
{
    char *line, *next;
    int rcode = 0;

    /*
     * Break into individual lines, and execute each line; terminate on
     * error.
     */
    next = cmd;
    line = cmd;
    while (*next) {
        if (*next == '\n') {
            *next = '\0';
            /* run only non-empty commands */
            if (*line) {
                DEBUG("** exec: \"%s\"\n", line);
                if (console_run_command(line, 0) < 0) {
                    rcode = 1;
                    break;
                }
            }
            line = next + 1;
        }
        ++next;
    }
    if (rcode == 0 && *line)
        rcode = (console_run_command(line, 0) < 0);

    return rcode;
}

int run_command(const char *cmd, int flag)
{
    if (console_run_command(cmd, flag) == -1)
        return 1;

    return 0;
}

int run_command_repeatable(const char *cmd, int flag)
{
    return console_run_command(cmd, flag);
}

int run_command_list(const char *cmd, int len, int flag)
{
    int need_buff = 1;
    char *buff = (char *)cmd;	/* cast away const */
    int rcode = 0;

    if (len == -1) {
        len = strlen(cmd);
        /* the built-in parser will change our string if it sees \n */
        need_buff = strchr(cmd, '\n') != NULL;
    }
    if (need_buff) {
        buff = balloc(len + 1);
        if (!buff)
            return 1;
        memcpy(buff, cmd, len);
        buff[len] = '\0';
    }

    rcode = console_run_command_list(buff, flag);

    if (need_buff)
        bfree(buff);

    return rcode;
}

void __console_loop(void)
{
    static char lastcommand[SYS_CBSIZE + 1] = { 0, };

    int len;
    int flag;
    int rc = 1;

    for (;;) {
        if (rc >= 0) {
            /* Saw enough of a valid command to
             * restart the timeout.
             */
            __reset_cmd_timeout();
        }
        len = console_readline(SYS_PROMPT);

        flag = 0;	/* assume no special flags for now */
        if (len > 0)
            strlcpy(lastcommand, console_buffer,
                SYS_CBSIZE + 1);
        else if (len == 0)
            flag |= CMD_FLAG_REPEAT;

        else if (len == -2) {
            /* -2 means timed out, retry autoboot
             */
            puts("\nTimed out waiting for command\n");

            /* Reinit board to run initialization code again */
            do_reboot();
        }

        if (len == -1)
            puts("<INTERRUPT>\n");
        else
            rc = run_command_repeatable(lastcommand, flag);

        if (rc <= 0) {
            /* invalid command or not repeatable, forget it */
            lastcommand[0] = 0;
        }
    }
}

int run_commandf(const char *fmt, ...)
{
    va_list args;
    int nbytes;

    va_start(args, fmt);
    /*
     * Limit the console_buffer space being used to SYS_CBSIZE,
     * because its last byte is used to fit the replacement of \0 by \n\0
     * in underlying hush parser
     */
    nbytes = vsnpr(console_buffer, SYS_CBSIZE, fmt, args);
    va_end(args);

    if (nbytes < 0) {
        DEBUG("I/O internal error occurred.\n");
        return -EIO;
    } else if (nbytes >= SYS_CBSIZE) {
        DEBUG("'fmt' size:%d exceeds the limit(%d)\n",
             nbytes, SYS_CBSIZE);
        return -ENOSPC;
    }
    return run_command(console_buffer, 0);
}

static void hypos_stamp(void)
{
    MSGI("                __________    _________  \n");
    MSGI(" /A__/A /A  /A / ___/_  _/A  / ___/ __ A \n");
    MSGI(" V  __ AV A_V AV___A / // /_/ ___AA __ / \n");
    MSGI("  V_A V_AV____/____/ V/ V___A____AV/  V  HYPOS\n");
    MSGI("\n");
}

int __bootfunc console_setup(void)
{
    /* TODO
     */
    hypos_stamp();

    if (!get_globl()->console_enable) {
        DEBUG("Console been disbale\n");
        force_kick_guests_up();
    } else {
        __console_loop();
    }

    return 0;
}
// --------------------------------------------------------------
