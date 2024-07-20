/**
 * Hustler's Project
 *
 * File:  radix.h
 * Date:  2024/07/20
 * Usage: radix tree implementation
 */

#ifndef _LIB_RADIX_H
#define _LIB_RADIX_H
// -------------------------------------------------------------
#include <bsp/rcu.h>


#define RADIX_TREE_INDIRECT_PTR	1

static inline int radix_tree_is_indirect_ptr(void *ptr)
{
	return (int)((unsigned long)ptr & RADIX_TREE_INDIRECT_PTR);
}

#define RADIX_TREE_MAP_SHIFT	6
#define RADIX_TREE_MAP_SIZE	(1UL << RADIX_TREE_MAP_SHIFT)
#define RADIX_TREE_MAP_MASK	(RADIX_TREE_MAP_SIZE-1)

struct radix_tree_node {
	unsigned int	height;		/* Height from the bottom */
	unsigned int	count;
	void *slots[RADIX_TREE_MAP_SIZE];
};

typedef struct radix_tree_node *radix_tree_alloc_fn_t(void *);
typedef void radix_tree_free_fn_t(struct radix_tree_node *, void *);

struct radix_tree_root {
	unsigned int		height;
	struct radix_tree_node *rnode;

	radix_tree_alloc_fn_t *node_alloc;
	radix_tree_free_fn_t *node_free;
	void *node_alloc_free_arg;
};

void radix_tree_init(struct radix_tree_root *root);
void radix_tree_set_alloc_callbacks(
	struct radix_tree_root *root,
	radix_tree_alloc_fn_t *node_alloc,
	radix_tree_free_fn_t *node_free,
	void *node_alloc_free_arg);

void radix_tree_destroy(
	struct radix_tree_root *root,
	void (*slot_free)(void *));

static inline void *radix_tree_deref_slot(void **pslot)
{
	return rcu_dereference(*pslot);
}

static inline int radix_tree_deref_retry(void *arg)
{
	return unlikely((unsigned long)arg & RADIX_TREE_INDIRECT_PTR);
}

static inline void radix_tree_replace_slot(void **pslot,
                                           void *item)
{
	BUG_ON(radix_tree_is_indirect_ptr(item));
	rcu_assign_pointer(*pslot, item);
}

static inline void *radix_tree_int_to_ptr(int val)
{
    long _ptr = ((long)val << 2) | 0x2l;
    ASSERT((_ptr >> 2) == val);
    return (void *)_ptr;
}

static inline int radix_tree_ptr_to_int(void *ptr)
{
    ASSERT(((long)ptr & 0x3) == 0x2);
    return (int)((long)ptr >> 2);
}

static inline void *radix_tree_ulong_to_ptr(unsigned long val)
{
    unsigned long ptr = (val << 2) | 0x2;
    ASSERT((ptr >> 2) == val);
    return (void *)ptr;
}

static inline unsigned long radix_tree_ptr_to_ulong(void *ptr)
{
    ASSERT(((unsigned long)ptr & 0x3) == 0x2);
    return (unsigned long)ptr >> 2;
}

int radix_tree_insert(struct radix_tree_root *,
                      unsigned long, void *);
void *radix_tree_lookup(struct radix_tree_root *,
                        unsigned long);
void **radix_tree_lookup_slot(struct radix_tree_root *,
                              unsigned long);
void *radix_tree_delete(struct radix_tree_root *,
                        unsigned long);
unsigned int
radix_tree_gang_lookup(struct radix_tree_root *root,
                       void **results,
                       unsigned long first_index,
                       unsigned int max_items);
unsigned int
radix_tree_gang_lookup_slot(struct radix_tree_root *root,
                            void ***results,
                            unsigned long first_index,
                            unsigned int max_items);
unsigned long radix_tree_next_hole(struct radix_tree_root *root,
				unsigned long index, unsigned long max_scan);
unsigned long radix_tree_prev_hole(struct radix_tree_root *root,
				unsigned long index, unsigned long max_scan);

// --------------------------------------------------------------
#endif /* _LIB_RADIX_H */
