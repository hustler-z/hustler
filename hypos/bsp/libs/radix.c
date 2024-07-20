/**
 * Hustler's Project
 *
 * File:  radix.c
 * Date:  2024/07/20
 * Usage:
 */

#include <lib/radix.h>
#include <lib/define.h>
#include <lib/strops.h>
#include <bsp/errno.h>
#include <bsp/hypmem.h>
#include <bsp/bootcore.h>
// --------------------------------------------------------------
struct radix_tree_path {
    struct radix_tree_node *node;
    int offset;
};

#define RADIX_TREE_INDEX_BITS  (8 * sizeof(unsigned long))
#define RADIX_TREE_MAX_PATH (DIV_ROUND_UP(RADIX_TREE_INDEX_BITS, \
					  RADIX_TREE_MAP_SHIFT))

static unsigned long
height_to_maxindex[RADIX_TREE_MAX_PATH + 1] __read_mostly;

static inline void *ptr_to_indirect(void *ptr)
{
    return (void *)((unsigned long)ptr | RADIX_TREE_INDIRECT_PTR);
}

static inline void *indirect_to_ptr(void *ptr)
{
    return (void *)((unsigned long)ptr & ~RADIX_TREE_INDIRECT_PTR);
}

struct rcu_node {
    struct radix_tree_node node;
    struct rcu_head rcu_head;
};

static struct radix_tree_node *rcu_node_alloc(void *arg)
{
    struct rcu_node *rcu_node = malloc(struct rcu_node);
    return rcu_node ? &rcu_node->node : NULL;
}

static void _rcu_node_free(struct rcu_head *head)
{
    struct rcu_node *rcu_node =
            container_of(head, struct rcu_node, rcu_head);
    free(rcu_node);
}

static void rcu_node_free(struct radix_tree_node *node, void *arg)
{
    struct rcu_node *rcu_node =
            container_of(node, struct rcu_node, node);
    call_rcu(&rcu_node->rcu_head, _rcu_node_free);
}

static struct radix_tree_node *radix_tree_node_alloc(
	struct radix_tree_root *root)
{
    struct radix_tree_node *ret;
    ret = root->node_alloc(root->node_alloc_free_arg);
    if (ret)
        memset(ret, 0, sizeof(*ret));
    return ret;
}

static void radix_tree_node_free(
	struct radix_tree_root *root, struct radix_tree_node *node)
{
    root->node_free(node, root->node_alloc_free_arg);
}

static inline
unsigned long radix_tree_maxindex(unsigned int height)
{
    return height_to_maxindex[height];
}

static int radix_tree_extend(struct radix_tree_root *root,
                             unsigned long index)
{
    struct radix_tree_node *node;
    unsigned int height;

    /* Figure out what the height should be.  */
    height = root->height + 1;
    while (index > radix_tree_maxindex(height))
        height++;

    if (root->rnode == NULL) {
        root->height = height;
        goto out;
    }

    do {
        unsigned int newheight;
        if (!(node = radix_tree_node_alloc(root)))
            return -ENOMEM;

        /* Increase the height.  */
        node->slots[0] = indirect_to_ptr(root->rnode);

        newheight = root->height+1;
        node->height = newheight;
        node->count = 1;
        node = ptr_to_indirect(node);
        rcu_assign_pointer(root->rnode, node);
        root->height = newheight;
    } while (height > root->height);
out:
    return 0;
}

int radix_tree_insert(struct radix_tree_root *root,
			unsigned long index, void *item)
{
    struct radix_tree_node *node = NULL, *slot;
    unsigned int height, shift;
    int offset;
    int error;

    BUG_ON(radix_tree_is_indirect_ptr(item));

    /* Make sure the tree is high enough.  */
    if (index > radix_tree_maxindex(root->height)) {
        error = radix_tree_extend(root, index);
        if (error)
            return error;
    }

    slot = indirect_to_ptr(root->rnode);

    height = root->height;
    shift = (height-1) * RADIX_TREE_MAP_SHIFT;

    offset = 0;			/* uninitialised var warning */
    while (height > 0) {
        if (slot == NULL) {
            /* Have to add a child node.  */
            if (!(slot = radix_tree_node_alloc(root)))
                return -ENOMEM;
            slot->height = height;
            if (node) {
                rcu_assign_pointer(node->slots[offset], slot);
                node->count++;
            } else
                rcu_assign_pointer(root->rnode,
                                   ptr_to_indirect(slot));
        }

        /* Go a level down */
        offset = (index >> shift) & RADIX_TREE_MAP_MASK;
        node = slot;
        slot = node->slots[offset];
        shift -= RADIX_TREE_MAP_SHIFT;
        height--;
    }

    if (slot != NULL)
        return -EEXIST;

    if (node) {
        node->count++;
        rcu_assign_pointer(node->slots[offset], item);
    } else {
        rcu_assign_pointer(root->rnode, item);
    }

    return 0;
}

static void *
radix_tree_lookup_element(struct radix_tree_root *root,
				          unsigned long index, int is_slot)
{
    unsigned int height, shift;
    struct radix_tree_node *node, **slot;

    node = rcu_dereference(root->rnode);
    if (node == NULL)
        return NULL;

    if (!radix_tree_is_indirect_ptr(node)) {
        if (index > 0)
            return NULL;
        return is_slot ? (void *)&root->rnode : node;
    }
    node = indirect_to_ptr(node);

    height = node->height;
    if (index > radix_tree_maxindex(height))
        return NULL;

    shift = (height-1) * RADIX_TREE_MAP_SHIFT;

    do {
        slot = (struct radix_tree_node **)
               (node->slots + ((index>>shift) &
                RADIX_TREE_MAP_MASK));
        node = rcu_dereference(*slot);
        if (node == NULL)
            return NULL;

        shift -= RADIX_TREE_MAP_SHIFT;
        height--;
    } while (height > 0);

    return is_slot ? (void *)slot : indirect_to_ptr(node);
}

void **radix_tree_lookup_slot(struct radix_tree_root *root,
                              unsigned long index)
{
    return (void **)radix_tree_lookup_element(root, index, 1);
}

void *radix_tree_lookup(struct radix_tree_root *root,
                        unsigned long index)
{
    return radix_tree_lookup_element(root, index, 0);
}

unsigned long radix_tree_next_hole(struct radix_tree_root *root,
                                   unsigned long index,
                                   unsigned long max_scan)
{
    unsigned long i;

    for (i = 0; i < max_scan; i++) {
        if (!radix_tree_lookup(root, index))
            break;
        index++;
        if (index == 0)
            break;
    }

    return index;
}

unsigned long radix_tree_prev_hole(struct radix_tree_root *root,
                                   unsigned long index,
                                   unsigned long max_scan)
{
    unsigned long i;

    for (i = 0; i < max_scan; i++) {
        if (!radix_tree_lookup(root, index))
            break;
        index--;
        if (index == ULONG_MAX)
            break;
    }

    return index;
}

static unsigned int
__lookup(struct radix_tree_node *slot,
         void ***results,
         unsigned long index,
         unsigned int max_items,
         unsigned long *next_index)
{
    unsigned int nr_found = 0;
    unsigned int shift, height;
    unsigned long i;

    height = slot->height;
    if (height == 0)
        goto out;
    shift = (height-1) * RADIX_TREE_MAP_SHIFT;

    for ( ; height > 1; height--) {
        i = (index >> shift) & RADIX_TREE_MAP_MASK;
        for (;;) {
            if (slot->slots[i] != NULL)
                break;
            index &= ~((1UL << shift) - 1);
            index += 1UL << shift;
            if (index == 0)
                goto out;
            i++;
            if (i == RADIX_TREE_MAP_SIZE)
                goto out;
        }

        shift -= RADIX_TREE_MAP_SHIFT;
        slot = rcu_dereference(slot->slots[i]);
        if (slot == NULL)
            goto out;
    }

    for (i = index & RADIX_TREE_MAP_MASK;
         i < RADIX_TREE_MAP_SIZE; i++) {
        index++;
        if (slot->slots[i]) {
            results[nr_found++] = &(slot->slots[i]);
            if (nr_found == max_items)
                goto out;
        }
    }
out:
    *next_index = index;
    return nr_found;
}

unsigned int
radix_tree_gang_lookup(struct radix_tree_root *root,
                       void **results,
                       unsigned long first_index,
                       unsigned int max_items)
{
    unsigned long max_index;
    struct radix_tree_node *node;
    unsigned long cur_index = first_index;
    unsigned int ret;

    node = rcu_dereference(root->rnode);
    if (!node)
        return 0;

    if (!radix_tree_is_indirect_ptr(node)) {
        if (first_index > 0)
            return 0;
        results[0] = node;
        return 1;
    }
    node = indirect_to_ptr(node);

    max_index = radix_tree_maxindex(node->height);

    ret = 0;
    while (ret < max_items) {
        unsigned int nr_found, slots_found, i;
        unsigned long next_index;

        if (cur_index > max_index)
            break;
        slots_found = __lookup(node, (void ***)results + ret,
                               cur_index, max_items - ret,
                               &next_index);
        nr_found = 0;
        for (i = 0; i < slots_found; i++) {
            struct radix_tree_node *slot;
            slot = *(((void ***)results)[ret + i]);
            if (!slot)
                continue;
            results[ret + nr_found] =
                indirect_to_ptr(rcu_dereference(slot));
            nr_found++;
        }
        ret += nr_found;
        if (next_index == 0)
            break;
        cur_index = next_index;
    }

    return ret;
}

unsigned int
radix_tree_gang_lookup_slot(struct radix_tree_root *root,
                            void ***results,
                            unsigned long first_index,
                            unsigned int max_items)
{
    unsigned long max_index;
    struct radix_tree_node *node;
    unsigned long cur_index = first_index;
    unsigned int ret;

    node = rcu_dereference(root->rnode);
    if (!node)
        return 0;

    if (!radix_tree_is_indirect_ptr(node)) {
        if (first_index > 0)
            return 0;
        results[0] = (void **)&root->rnode;
        return 1;
    }
    node = indirect_to_ptr(node);

    max_index = radix_tree_maxindex(node->height);

    ret = 0;
    while (ret < max_items) {
        unsigned int slots_found;
        unsigned long next_index;	/* Index of next search */

        if (cur_index > max_index)
            break;
        slots_found = __lookup(node, results + ret, cur_index,
                    max_items - ret, &next_index);
        ret += slots_found;
        if (next_index == 0)
            break;
        cur_index = next_index;
    }

    return ret;
}

static inline void radix_tree_shrink(struct radix_tree_root *root)
{
    while (root->height > 0) {
        struct radix_tree_node *to_free = root->rnode;
        void *newptr;

        BUG_ON(!radix_tree_is_indirect_ptr(to_free));
        to_free = indirect_to_ptr(to_free);

        if (to_free->count != 1)
            break;
        if (!to_free->slots[0])
            break;

        newptr = to_free->slots[0];
        if (root->height > 1)
            newptr = ptr_to_indirect(newptr);
        root->rnode = newptr;
        root->height--;

        if (root->height == 0)
            *((unsigned long *)&to_free->slots[0]) |=
                        RADIX_TREE_INDIRECT_PTR;

        radix_tree_node_free(root, to_free);
    }
}

void *radix_tree_delete(struct radix_tree_root *root,
                        unsigned long index)
{
    struct radix_tree_path path[RADIX_TREE_MAX_PATH + 1],
                           *pathp = path;
    struct radix_tree_node *slot = NULL;
    struct radix_tree_node *to_free;
    unsigned int height, shift;
    int offset;

    height = root->height;
    if (index > radix_tree_maxindex(height))
        goto out;

    slot = root->rnode;
    if (height == 0) {
        root->rnode = NULL;
        goto out;
    }
    slot = indirect_to_ptr(slot);

    shift = (height - 1) * RADIX_TREE_MAP_SHIFT;
    pathp->node = NULL;

    do {
        if (slot == NULL)
            goto out;

        pathp++;
        offset = (index >> shift) & RADIX_TREE_MAP_MASK;
        pathp->offset = offset;
        pathp->node = slot;
        slot = slot->slots[offset];
        shift -= RADIX_TREE_MAP_SHIFT;
        height--;
    } while (height > 0);

    if (slot == NULL)
        goto out;

    to_free = NULL;
    while (pathp->node) {
        pathp->node->slots[pathp->offset] = NULL;
        pathp->node->count--;

        if (to_free)
            radix_tree_node_free(root, to_free);

        if (pathp->node->count) {
            if (pathp->node == indirect_to_ptr(root->rnode))
                radix_tree_shrink(root);
            goto out;
        }

        to_free = pathp->node;
        pathp--;

    }
    root->height = 0;
    root->rnode = NULL;
    if (to_free)
        radix_tree_node_free(root, to_free);

out:
    return slot;
}

static void
radix_tree_node_destroy(struct radix_tree_root *root,
                        struct radix_tree_node *node,
                        void (*slot_free)(void *))
{
    int i;

    for (i = 0; i < RADIX_TREE_MAP_SIZE; i++) {
        struct radix_tree_node *slot = node->slots[i];
        BUG_ON(radix_tree_is_indirect_ptr(slot));
        if (slot == NULL)
            continue;
        if (node->height == 1) {
            if (slot_free)
                slot_free(slot);
        } else {
            radix_tree_node_destroy(root, slot, slot_free);
        }
    }

    radix_tree_node_free(root, node);
}

void radix_tree_destroy(struct radix_tree_root *root,
                        void (*slot_free)(void *))
{
    struct radix_tree_node *node = root->rnode;
    if (node == NULL)
        return;
    if (!radix_tree_is_indirect_ptr(node)) {
        if (slot_free)
            slot_free(node);
    } else {
        node = indirect_to_ptr(node);
        radix_tree_node_destroy(root, node, slot_free);
    }
    radix_tree_init(root);
}

void radix_tree_init(struct radix_tree_root *root)
{
    memset(root, 0, sizeof(*root));
    root->node_alloc = rcu_node_alloc;
    root->node_free = rcu_node_free;
}

void radix_tree_set_alloc_callbacks(struct radix_tree_root *root,
                                    radix_tree_alloc_fn_t *node_alloc,
                                    radix_tree_free_fn_t *node_free,
                                    void *node_alloc_free_arg)
{
    root->node_alloc = node_alloc;
    root->node_free = node_free;
    root->node_alloc_free_arg = node_alloc_free_arg;
}

static __bootfunc unsigned long __maxindex(unsigned int height)
{
    unsigned int width = height * RADIX_TREE_MAP_SHIFT;
    int shift = RADIX_TREE_INDEX_BITS - width;

    if (shift < 0)
        return ~0UL;
    if (shift >= BITS_PER_LONG)
        return 0UL;
    return ~0UL >> shift;
}

static int __bootfunc radix_tree_init_maxindex(void)
{
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(height_to_maxindex); i++)
        height_to_maxindex[i] = __maxindex(i);

    return 0;
}
__bootcall(radix_tree_init_maxindex);
// --------------------------------------------------------------
