/**
 * Hustler's Project
 *
 * File:  guest.c
 * Date:  2024/07/23
 * Usage: Guest OS related Implementation
 */

#include <org/vttbl.h>
#include <bsp/config.h>

#if IS_IMPLEMENTED(__GUEST_IMPL)
// --------------------------------------------------------------

#define COPY_FLUSH_DCACHE   (1U << 0)
#define COPY_FROM_GUEST     (0U << 1)
#define COPY_TO_GUEST       (1U << 1)
#define COPY_IPA            (0U << 2)
#define COPY_LINEAR         (1U << 2)

typedef union {
    struct {
        struct vcpu *v;
    } gva;

    struct {
        struct hypos *h;
    } gpa;
} copy_info_t;

#define GVA_INFO(vcpu)  ((copy_info_t) { .gva = { vcpu } })
#define GPA_INFO(hypos) ((copy_info_t) { .gpa = { hypos } })

static struct page *
translate_get_page(copy_info_t info, u64 addr,
                   bool linear, bool write)
{
    vttbl_type_t vttblt;
    struct page_info *page;

    if (linear)
        return get_page_from_gva(info.gva.v, addr,
                                 write ? GVP_WRITE : GVP_READ);

    page = get_page_from_gfn(info.gpa.d, paddr_to_pfn(addr),
                             &vttblt, VTTBL_ALLOC);

    if (!page)
        return NULL;

    if (!vttbl_is_ram(vttblt)) {
        put_page(page);
        return NULL;
    }

    return page;
}

static unsigned long copy_guest(void *buf, u64 addr,
                                unsigned int len,
                                copy_info_t info,
                                unsigned int flags)
{
    unsigned int offset = addr & ~PAGE_MASK;

    BUILD_BUG_ON((sizeof(addr)) < sizeof(vaddr_t));
    BUILD_BUG_ON((sizeof(addr)) < sizeof(paddr_t));

    while (len) {
        void *p;
        unsigned int size = min(len,
                                (unsigned int)PAGE_SIZE - offset);
        struct page *page;

        page = translate_get_page(info, addr, flags & COPY_LINEAR,
                                  flags & COPY_TO_GUEST);
        if (page == NULL)
            return len;

        p = __map_hypos_page(page);
        p += offset;
        if (flags & COPY_TO_GUEST) {
            if (buf)
                memcpy(p, buf, size);
            else
                memset(p, 0, size);
        } else
            memcpy(buf, p, size);

        if (flags & COPY_FLUSH_DCACHE)
            clean_dcache_va_range(p, size);

        unmap_hypos_page(p - offset);
        put_page(page);
        len -= size;
        buf += size;
        addr += size;

        offset = 0;
    }

    return 0;
}

unsigned long raw_copy_to_guest(void *to, const void *from,
                                unsigned int len)
{
    return copy_guest((void *)from, (vaddr_t)to, len,
                      GVA_INFO(current), COPY_TO_GUEST |
                      COPY_LINEAR);
}

unsigned long raw_copy_to_guest_flush_dcache(void *to,
                                             const void *from,
                                             unsigned int len)
{
    return copy_guest((void *)from, (vaddr_t)to,
                      len, GVA_INFO(current),
                      COPY_TO_GUEST | COPY_FLUSH_DCACHE
                      | COPY_LINEAR);
}

unsigned long raw_clear_guest(void *to, unsigned int len)
{
    return copy_guest(NULL, (vaddr_t)to, len, GVA_INFO(current),
                      COPY_TO_GUEST | COPY_LINEAR);
}

unsigned long raw_copy_from_guest(void *to,
                                  const void *from,
                                  unsigned int len)
{
    return copy_guest(to, (vaddr_t)from, len, GVA_INFO(current),
                      COPY_FROM_GUEST | COPY_LINEAR);
}

unsigned long copy_to_guest_phys_flush_dcache(struct hypos *d,
                                              gpa_t gpa,
                                              void *buf,
                                              unsigned int len)
{
    return copy_guest(buf, gpa, len, GPA_INFO(d),
                      COPY_TO_GUEST | COPY_IPA |
                      COPY_FLUSH_DCACHE);
}

int access_guest_memory_by_gpa(struct hypos *h, gpa_t gpa,
                               void *buf, u32 size,
                               bool is_write)
{
    unsigned long left;
    int flags = COPY_IPA;

    flags |= is_write ? COPY_TO_GUEST : COPY_FROM_GUEST;

    left = copy_guest(buf, gpa, size, GPA_INFO(h), flags);

    return (!left) ? 0 : -EINVAL;
}

char *safe_copy_string_from_guest(__GUEST_HANDLE(char)__buf,
                                  size_t size, size_t max_size)
{
    char *tmp;

    if (size > max_size)
        return ERR_PTR(-ENOBUFS);

    tmp = malloc_array(char, size + 1);
    if (!tmp)
        return ERR_PTR(-ENOMEM);

    if (copy_from_guest(tmp, __buf, size)) {
        free(tmp);
        return ERR_PTR(-EFAULT);
    }
    tmp[size] = '\0';

    return tmp;
}

// --------------------------------------------------------------
#else

int access_guest_memory_by_gpa(struct hypos *h, gpa_t gpa,
                               void *buf, u32 size,
                               bool is_write)
{
    return 0;
}

// --------------------------------------------------------------
#endif
