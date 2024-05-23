/**
 * Hustler's Project
 *
 * File:  bsp_cmd.h
 * Date:  2024/05/20
 * Usage: general commands interact with console
 */

#ifndef _BSP_CMD_H
#define _BSP_CMD_H
// ------------------------------------------------------------------------

struct bsp_cmd_ops {
    void (*meminfo)(void);
    void (*cpuinfo)(void);
    void (*hypinfo)(void);
    void (*guestup)(int guestid);
};

void bsp_cmd_register(struct bsp_cmd_ops *cmd_ops);

// ------------------------------------------------------------------------
#endif /* _BSP_CMD_H */
