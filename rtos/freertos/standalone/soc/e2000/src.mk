
SOC_TYPE_NAME := $(subst ",,$(CONFIG_TARGET_TYPE_NAME))

SOC_CSRCS += $(SOC_TYPE_NAME)/fmmu_table.c\
             $(SOC_TYPE_NAME)/fcpu_affinity_mask.c
