/**
 * Hustler's Project
 *
 * File:  numa.h
 * Date:  2024/07/04
 * Usage:
 */

#ifndef _COMMON_NUMA_H
#define _COMMON_NUMA_H
// --------------------------------------------------------------
#include <common/type.h>

typedef u8              nid_t;

struct node_data {
    unsigned long node_start_pfn;
    unsigned long node_spanned_pages;
};

#define MAX_NUMNODES          (0)
#define cpu_to_node(cpu)      (0)

// --------------------------------------------------------------
#endif /* _COMMON_NUMA_H */
