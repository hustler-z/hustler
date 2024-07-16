/**
 * Hustler's Project
 *
 * File:  numa.h
 * Date:  2024/07/04
 * Usage:
 */

#ifndef _BSP_NUMA_H
#define _BSP_NUMA_H
// --------------------------------------------------------------
#include <bsp/type.h>

typedef u8              nid_t;

struct node_data {
    unsigned long node_start_pfn;
    unsigned long node_spanned_pages;
};

#define MAX_NUMNODES          (0)
#define cpu_to_node(cpu)      (0)

// --------------------------------------------------------------
#endif /* _BSP_NUMA_H */
