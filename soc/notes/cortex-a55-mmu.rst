+------------------------------------------------------------------------------+
| CORTEX A55 MEMORY MANAGEMENT UNIT                                            |
+------------------------------------------------------------------------------+

The Translation Lookaside Buffer (TLB) is a cache of recently executed page
translations within the MMU. The Cortex®-A55 core implements a two-level TLB
structure. The L2 TLB stores all page sizes and is responsible for breaking
these down into smaller pages when required for the data-side or instruction
-side L1 TLB.

When the Cortex®-A55 core generates a memory access, the MMU:
1. Performs a lookup for the requested VA and current translation regime in
   the relevant instruction or data L1 TLB.
2. If there is a miss in the relevant L1 TLB, the MMU performs a lookup for
   the requested VA,current ASID, current VMID, and translation regime in the
   L2 TLB.
3. If there is a miss in the L2 TLB, the MMU performs a hardware translation
   table walk.

In the case of an L2 TLB miss, the hardware does a translation table walk as
long as the MMU is enabled, and the translation using the base register has
not been disabled.

If the translation table walk is disabled for a particular base register, the
core returns a Translation Fault. If the TLB finds a matching entry, it uses
the information in the entry as follows.

The access permission bits and the domain determine if the access is permitted.
If the matching entry does not pass the permission checks, the MMU signals a
Permission fault.


--------------------------------------------------------------------------------
