/**
 * Hustler's Project
 *
 * File:  pci.c
 * Date:  2024/07/19
 * Usage:
 */

#include <bsp/config.h>
// --------------------------------------------------------------

/* --------------------------------------------------------------
 * XXX: PCI (Peripheral Component Interconnect)
 *
 *
 *
 *
 * --------------------------------------------------------------
 */

#if IS_IMPLEMENTED(__PCI_IMPL)
// --------------------------------------------------------------
#include <org/pci.h>
#include <bsp/vmap.h>

#define INVALID_VALUE (~0U)
#define PCI_ERR_VALUE(len) GENMASK(0, len * 8)

int pci_generic_config_read(struct pci_host_bridge *bridge,
                            pci_sbdf_t sbdf,
                            u32 reg, u32 len, u32 *value)
{
    void __iomem *addr = bridge->ops->map_bus(bridge, sbdf, reg);

    if (!addr) {
        *value = INVALID_VALUE;
        return -ENODEV;
    }

    switch (len) {
    case 1:
        *value = readb(addr);
        break;
    case 2:
        *value = readw(addr);
        break;
    case 4:
        *value = readl(addr);
        break;
    default:
        ASSERT_UNREACHABLE();
    }

    return 0;
}

int pci_generic_config_write(struct pci_host_bridge *bridge,
                             pci_sbdf_t sbdf,
                             u32 reg, u32 len, u32 value)
{
    void __iomem *addr = bridge->ops->map_bus(bridge, sbdf, reg);

    if (!addr)
        return -ENODEV;

    switch (len) {
    case 1:
        writeb(value, addr);
        break;
    case 2:
        writew(value, addr);
        break;
    case 4:
        writel(value, addr);
        break;
    default:
        ASSERT_UNREACHABLE();
    }

    return 0;
}

static u32 pci_config_read(pci_sbdf_t sbdf, unsigned int reg,
                                unsigned int len)
{
    u32 val = PCI_ERR_VALUE(len);
    struct pci_host_bridge *bridge =
            pci_find_host_bridge(sbdf.seg, sbdf.bus);

    if (unlikely(!bridge))
        return val;

    if (unlikely(!bridge->ops->read))
        return val;

    bridge->ops->read(bridge, sbdf, reg, len, &val);

    return val;
}

static void pci_config_write(pci_sbdf_t sbdf, unsigned int reg,
                             unsigned int len, u32 val)
{
    struct pci_host_bridge *bridge =
            pci_find_host_bridge(sbdf.seg, sbdf.bus);

    if (unlikely(!bridge))
        return;

    if (unlikely(!bridge->ops->write))
        return;

    bridge->ops->write(bridge, sbdf, reg, len, val);
}

#define PCI_OP_WRITE(size, type)                            \
    void pci_conf_write##size(pci_sbdf_t sbdf,              \
                              unsigned int reg, type val)   \
{                                                           \
    pci_config_write(sbdf, reg, size / 8, val);             \
}

#define PCI_OP_READ(size, type)                             \
    type pci_conf_read##size(pci_sbdf_t sbdf,               \
                              unsigned int reg)             \
{                                                           \
    return pci_config_read(sbdf, reg, size / 8);            \
}

PCI_OP_READ(8, u8)
PCI_OP_READ(16, u16)
PCI_OP_READ(32, u32)
PCI_OP_WRITE(8, u8)
PCI_OP_WRITE(16, u16)
PCI_OP_WRITE(32, u32)

struct pdev_bar_check
{
    paddr_t start;
    paddr_t end;
    bool is_valid;
};

static LIST_HEAD(pci_host_bridges);

static atomic_t hypos_nr = ATOMIC_INIT(-1);

static int use_dt_hyposs = -1;

static inline void __iomem *pci_remap_cfgspace(paddr_t start,
                                               size_t len)
{
    return ioremap_nocache(start, len);
}

static void pci_ecam_free(struct pci_config_window *cfg)
{
    if (cfg->win)
        iounmap(cfg->win);

    xfree(cfg);
}

static struct pci_config_window * __bootfunc
gen_pci_init(struct __device_node *dev,
             const struct pci_ecam_ops *ops)
{
    int err, cfg_reg_idx;
    u32 bus_range[2];
    paddr_t addr, size;
    struct pci_config_window *cfg;

    cfg = zalloc(struct pci_config_window);
    if ( !cfg )
        return NULL;

    err = __property_read_u32_array(dev, "bus-range", bus_range,
                                     ARRAY_SIZE(bus_range));
    if (err) {
        cfg->busn_start = 0;
        cfg->busn_end = 0xFF;
        MSGI("%s: No bus range found for pci controller\n",
               __node_full_name(dev));
    } else {
        cfg->busn_start = bus_range[0];
        cfg->busn_end = bus_range[1];
        if (cfg->busn_end > cfg->busn_start + 0xFF)
            cfg->busn_end = cfg->busn_start + 0xFF;
    }

    if (ops->cfg_reg_index) {
        cfg_reg_idx = ops->cfg_reg_index(dev);
        if (cfg_reg_idx < 0)
            goto err_exit;
    } else
        cfg_reg_idx = 0;

    err = __device_get_paddr(dev, cfg_reg_idx, &addr, &size);
    if (err)
        goto err_exit;

    cfg->phys_addr = addr;
    cfg->size = size;

    cfg->win = pci_remap_cfgspace(cfg->phys_addr, cfg->size);
    if (!cfg->win) {
        MSGI("ECAM ioremap failed\n");
        goto err_exit;
    }
    MSGI("ECAM at [mem 0x%016lx-0x%016lx] for [bus %x-%x]\n",
            cfg->phys_addr, cfg->phys_addr + cfg->size - 1,
            cfg->busn_start, cfg->busn_end);

    if (ops->init) {
        err = ops->init(cfg);
        if (err)
            goto err_exit;
    }

    return cfg;

err_exit:
    pci_ecam_free(cfg);

    return NULL;
}

struct pci_host_bridge *pci_alloc_host_bridge(void)
{
    struct pci_host_bridge *bridge =
            zalloc(struct pci_host_bridge);

    if (!bridge)
        return NULL;

    INIT_LIST_HEAD(&bridge->node);

    return bridge;
}

void pci_add_host_bridge(struct pci_host_bridge *bridge)
{
    list_add_tail(&bridge->node, &pci_host_bridges);
}

int pci_get_new_hypos_nr(void)
{
    if (use_dt_hyposs)
        return -1;

    return atomic_inc_return(&hypos_nr);
}

static int pci_bus_find_hypos_nr(struct __device_node *dev)
{
    int hypos;

    hypos = __get_pci_hypos_nr(dev);

    if (hypos >= 0 && use_dt_hyposs) {
        use_dt_hyposs = 1;
    } else if (hypos < 0 && use_dt_hyposs != 1) {
        use_dt_hyposs = 0;
        hypos = pci_get_new_hypos_nr();
    } else {
        hypos = -1;
    }

    return hypos;
}

int pci_host_common_probe(struct __device_node *dev,
                          const struct pci_ecam_ops *ops)
{
    struct pci_host_bridge *bridge;
    struct pci_config_window *cfg;
    int err;
    int hypos;

    if (dt_device_for_passthrough(dev))
        return 0;

    bridge = pci_alloc_host_bridge();
    if (!bridge)
        return -ENOMEM;

    cfg = gen_pci_init(dev, ops);
    if (!cfg) {
        err = -ENOMEM;
        goto err_exit;
    }

    bridge->dt_node = dev;
    bridge->cfg = cfg;
    bridge->ops = &ops->pci_ops;

    hypos = pci_bus_find_hypos_nr(dev);
    if (hypos < 0) {
        MSGI("Inconsistent \"linux,pci-hypos\" property in DT\n");
        BUG();
    }
    bridge->segment = hypos;
    pci_add_host_bridge(bridge);

    return 0;

err_exit:
    free(bridge);

    return err;
}

const struct __device_node *
pci_find_host_bridge_node(const struct pci_dev *pdev)
{
    struct pci_host_bridge *bridge;

    bridge = pci_find_host_bridge(pdev->seg, pdev->bus);
    if (unlikely(!bridge)) {
        MSGI("Unable to find PCI bridge for %pp\n", &pdev->sbdf);
        return NULL;
    }
    return bridge->dt_node;
}

struct pci_host_bridge *pci_find_host_bridge(u16 segment,
                                             u8 bus)
{
    struct pci_host_bridge *bridge;

    list_for_each_entry(bridge, &pci_host_bridges, node) {
        if (bridge->segment != segment)
            continue;
        if ((bus < bridge->cfg->busn_start) ||
            (bus > bridge->cfg->busn_end))
            continue;
        return bridge;
    }

    return NULL;
}

int pci_get_host_bridge_segment(const struct __device_node *node,
                                u16 *segment)
{
    struct pci_host_bridge *bridge;

    list_for_each_entry(bridge, &pci_host_bridges, node) {
        if (bridge->dt_node != node)
            continue;

        *segment = bridge->segment;
        return 0;
    }

    return -EINVAL;
}

int pci_host_iterate_bridges_and_count(struct hypos *d,
                                       int (*cb)(struct hypos *d,
                                                 struct pci_host_bridge *bridge))
{
    struct pci_host_bridge *bridge;
    int count = 0;

    list_for_each_entry(bridge, &pci_host_bridges, node) {
        int ret;

        ret = cb(d, bridge);
        if (ret < 0)
            return ret;
        count += ret;
    }
    return count;
}

int __bootfunc pci_host_bridge_mappings(struct hypos *d)
{
    struct pci_host_bridge *bridge;
    struct map_range_data mr_data = {
        .d = d,
        .vttblt = vttbl_mmio_direct_dev,
        .skip_mapping = false
    };

    list_for_each_entry(bridge, &pci_host_bridges, node) {
        const struct __device_node *dev = bridge->dt_node;
        unsigned int i;

        for (i = 0; i < __number_of_address(dev); i++) {
            paddr_t addr, size;
            int err;

            err = __device_get_paddr(dev, i, &addr, &size);
            if (err) {
                MSGI("Unable to retrieve address range index=%u for %s\n",
                       i, __node_full_name(dev));
                return err;
            }

            if (bridge->ops->need_vttbl_hwdom_mapping(d, bridge, addr)) {
                err = map_range_to_hypos(dev, addr, size, &mr_data);
                if (err)
                    return err;
            }
        }
    }

    return 0;
}

static int is_bar_valid(const struct __device_node *dev,
                        u64 addr, u64 len, void *data)
{
    struct pdev_bar_check *bar_data = data;
    paddr_t s = bar_data->start;
    paddr_t e = bar_data->end;

    if ((s >= addr) && (e <= (addr + len - 1)))
        bar_data->is_valid =  true;

    return 0;
}

bool pci_check_bar(const struct pci_dev *pdev,
                   hfn_t start, hfn_t end)
{
    int ret;
    const struct __device_node *dt_node;
    paddr_t s = hfn_to_pa(start);
    paddr_t e = hfn_to_pa(hfn_add(end, 1)) - 1;
    struct pdev_bar_check bar_data =  {
        .start = s,
        .end = e,
        .is_valid = false
    };

    if (s > e)
        return false;

    __node = pci_find_host_bridge_node(pdev);
    if (!dt_node)
        return false;

    ret = __for_each_range(dt_node, &is_bar_valid, &bar_data);
    if (ret < 0)
        return false;

    return bar_data.is_valid;
}

void __iomem *pci_ecam_map_bus(struct pci_host_bridge *bridge,
                               pci_sbdf_t sbdf, u32 where)
{
    const struct pci_config_window *cfg = bridge->cfg;
    const struct pci_ecam_ops *ops =
        container_of(bridge->ops, const struct pci_ecam_ops, pci_ops);
    unsigned int devfn_shift = ops->bus_shift - 8;
    void __iomem *base;
    unsigned int busn = sbdf.bus;

    if (busn < cfg->busn_start || busn > cfg->busn_end)
        return NULL;

    busn -= cfg->busn_start;
    base = cfg->win + (busn << ops->bus_shift);

    return base + (sbdf.devfn << devfn_shift) + where;
}

bool __bootfunc
pci_ecam_need_vttbl_hw_map(struct hypos *d,
                           struct pci_host_bridge *bridge,
                           u64 addr)
{
    struct pci_config_window *cfg = bridge->cfg;

    return cfg->phys_addr != addr;
}

/* ECAM ops */
const struct pci_ecam_ops pci_generic_ecam_ops = {
    .bus_shift  = 20,
    .pci_ops    = {
        .map_bus                = pci_ecam_map_bus,
        .read                   = pci_generic_config_read,
        .write                  = pci_generic_config_write,
        .need_vttbl_hw_map = pci_ecam_need_vttbl_hw_map,
    }
};

// --------------------------------------------------------------
#endif
