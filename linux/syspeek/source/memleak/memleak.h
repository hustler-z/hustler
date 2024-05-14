#ifndef __MEMLEAK_H
#define __MEMLEAK_H

#define ALLOCS_MAX_ENTRIES 1000000
#define COMBINED_ALLOCS_MAX_ENTRIES 10240

struct alloc_info {
	__u64 size; /* size of allocated memory */
	__u64 timestamp_ns; /* timestamp when allocation occurs in nanoseconds */
	int stack_id; /* call stack ID when allocation occurs */
};

union combined_alloc_info {
	struct {
		__u64 total_size : 40; /* total size of all unreleased allocations */
		__u64 number_of_allocs : 24; /* total number of unreleased allocations */
	};
	__u64 bits; /* bitwise representation of the structure */
};

#endif /* __MEMLEAK_H */
