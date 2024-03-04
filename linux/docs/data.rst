+------------------------------------------------------------------------------+
| LINUX KERNEL DATA STRUCTRUES                                                 |
+------------------------------------------------------------------------------+

- RB-TREE -

--------------------------------------------------------------------------------
- XARRAY -

Mainly used for page cache.

DEFINE_XARRAY()
xa_init()

xa_store()
xa_load()
xa_erase()

xa_insert()
xa_alloc() => Finds an empty entry in @xa between @limit.min and @limit.max,
              stores the index into the @id pointer, then stores the entry
              at that index.

xa_find()

xa_reserve()
xa_release()

--------------------------------------------------------------------------------
- MAPLE TREE -

Mainly used for tracking of the virtual memory areas.

The Maple Tree is a B-Tree data type which is optimized for storing
non-overlapping ranges, including ranges of size 1. The Maple Tree
can store values between 0 and ULONG_MAX.

DEFINE_MTREE()
mt_init()

mt_find()

mtree_store()
mtree_insert()
mtree_erase()

--------------------------------------------------------------------------------
