
SOC_CSRCS += fcpu_info.c \
			fearly_uart.c
			

ifdef CONFIG_USE_SPINLOCK
SOC_CSRCS += fsmp.c
endif
