ifdef CONFIG_USE_I2S
ifdef CONFIG_USE_ES8336
DRIVERS_CSRCS += \
    fes8336.c\

endif
ifdef CONFIG_USE_FI2S
DRIVERS_CSRCS += \
    fi2s.c\
    fi2s_g.c\
    fi2s_intr.c\
    fi2s_sinit.c
endif

endif

