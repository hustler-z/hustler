/**
 * Hustler's Project
 *
 * File:  numa.c
 * Date:  2024/07/04
 * Usage:
 */

#include <bsp/numa.h>
#include <bsp/cpu.h>

// --------------------------------------------------------------
unsigned int memnode_shift;
unsigned long memnode_mapsize;
nid_t *memnode_map;
struct node_data node_data[NR_CPUS];
// --------------------------------------------------------------
