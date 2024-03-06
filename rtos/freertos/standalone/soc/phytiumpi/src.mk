

SOC_TYPE_NAME := $(subst ",,$(CONFIG_TARGET_TYPE_NAME))

SOC_CSRCS += \
    fmmu_table.c\
    fcpu_affinity_mask.c
