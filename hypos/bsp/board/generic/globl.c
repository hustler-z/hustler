/**
 * Hustler's Project
 *
 * File:  global.c
 * Date:  2024/05/22
 * Usage:
 */


#include <asm-generic/globl.h>
#include <asm-generic/section.h>
#include <lib/list.h>
// --------------------------------------------------------------
struct hypos_globl *glb;

/* The one and only hypos global data tracker: glb
 */
static struct hypos_globl boot_glb = {
    .console_enable = true,
};

/* The one and only periodic work list: glb_pw_list_head
 */
static struct hlist_head glb_pw_list_head;

static void set_glb_pw_list(struct hypos_globl *glb,
        struct hlist_head *pw_list)
{
    glb->pw_list = pw_list;
}

int __bootfunc glb_setup(void)
{
    glb = &boot_glb;

    INIT_HLIST_HEAD(&glb_pw_list_head);
    set_glb_pw_list(glb, &glb_pw_list_head);

    glb->flags |= GLB_INITIALIZED;

    return 0;
};

bool glb_is_initialized(void)
{
    if (!glb || !(glb->flags & GLB_INITIALIZED))
        return false;
    return true;
}
// --------------------------------------------------------------
