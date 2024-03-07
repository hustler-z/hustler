#ifndef SDK_CONFIG_H__
#define SDK_CONFIG_H__

#define CONFIG_USE_BAREMETAL

/* Arch configuration */

#define CONFIG_TARGET_ARMv8
#define CONFIG_ARCH_NAME "armv8"

/* Arm architecture configuration */

/* CONFIG_ARCH_ARMV8_AARCH64 is not set */
#define CONFIG_ARCH_ARMV8_AARCH32

/* Compiler configuration */

#define CONFIG_ARM_GCC_SELECT
/* CONFIG_ARM_CLANG_SELECT is not set */
#define CONFIG_TOOLCHAIN_NAME "gcc"
#define CONFIG_TARGET_ARMV8_AARCH32
#define CONFIG_ARCH_EXECUTION_STATE "aarch32"

/* Fpu configuration */

#define CONFIG_CRYPTO_NEON_FP_ARMV8
/* CONFIG_VFPV4 is not set */
/* CONFIG_VFPV4_D16 is not set */
/* CONFIG_VFPV3 is not set */
/* CONFIG_VFPV3_D16 is not set */
#define CONFIG_ARM_MFPU "crypto-neon-fp-armv8"
#define CONFIG_MFLOAT_ABI_HARD
/* CONFIG_MFLOAT_ABI_SOFTFP is not set */
#define CONFIG_ARM_MFLOAT_ABI "hard"
/* end of Fpu configuration */
/* end of Compiler configuration */
#define CONFIG_USE_CACHE
#define CONFIG_USE_MMU
#define CONFIG_USE_AARCH64_L1_TO_AARCH32
/* end of Arm architecture configuration */
/* end of Arch configuration */

/* Soc configuration */

/* CONFIG_TARGET_PHYTIUMPI is not set */
/* CONFIG_TARGET_E2000Q is not set */
#define CONFIG_TARGET_E2000D
/* CONFIG_TARGET_E2000S is not set */
/* CONFIG_TARGET_FT2004 is not set */
/* CONFIG_TARGET_D2000 is not set */
#define CONFIG_SOC_NAME "e2000"
#define CONFIG_TARGET_TYPE_NAME "d"
#define CONFIG_SOC_CORE_NUM 2
#define CONFIG_F32BIT_MEMORY_ADDRESS 0x80000000
#define CONFIG_F32BIT_MEMORY_LENGTH 0x80000000
#define CONFIG_F64BIT_MEMORY_ADDRESS 0x2000000000
#define CONFIG_F64BIT_MEMORY_LENGTH 0x800000000
#define CONFIG_TARGET_E2000
/* CONFIG_USE_SPINLOCK is not set */
#define CONFIG_DEFAULT_DEBUG_PRINT_UART1
/* CONFIG_DEFAULT_DEBUG_PRINT_UART0 is not set */
/* CONFIG_DEFAULT_DEBUG_PRINT_UART2 is not set */
/* end of Soc configuration */

/* Board Configuration */

#define CONFIG_E2000D_DEMO_BOARD
#define CONFIG_BOARD_NAME "demo"

/* IO mux configuration when board start up */

/* CONFIG_USE_SPI_IOPAD is not set */
/* CONFIG_USE_GPIO_IOPAD is not set */
/* CONFIG_USE_CAN_IOPAD is not set */
/* CONFIG_USE_QSPI_IOPAD is not set */
/* CONFIG_USE_PWM_IOPAD is not set */
/* CONFIG_USE_ADC_IOPAD is not set */
/* CONFIG_USE_MIO_IOPAD is not set */
/* CONFIG_USE_TACHO_IOPAD is not set */
/* CONFIG_USE_UART_IOPAD is not set */
/* CONFIG_USE_THIRD_PARTY_IOPAD is not set */
/* end of IO mux configuration when board start up */
/* CONFIG_CUS_DEMO_BOARD is not set */

/* Build project name */

#define CONFIG_TARGET_NAME "ddma_spim"
/* end of Build project name */
/* end of Board Configuration */

/* Sdk common configuration */

/* CONFIG_LOG_VERBOS is not set */
/* CONFIG_LOG_DEBUG is not set */
/* CONFIG_LOG_INFO is not set */
/* CONFIG_LOG_WARN is not set */
#define CONFIG_LOG_ERROR
/* CONFIG_LOG_NONE is not set */
#define CONFIG_LOG_EXTRA_INFO
/* CONFIG_LOG_DISPALY_CORE_NUM is not set */
/* CONFIG_BOOTUP_DEBUG_PRINTS is not set */
#define CONFIG_USE_DEFAULT_INTERRUPT_CONFIG
#define CONFIG_INTERRUPT_ROLE_MASTER
/* CONFIG_INTERRUPT_ROLE_SLAVE is not set */
/* end of Sdk common configuration */

/* Image information configuration */

/* CONFIG_IMAGE_INFO is not set */
/* end of Image information configuration */

/* Drivers configuration */

#define CONFIG_USE_IOMUX
/* CONFIG_ENABLE_IOCTRL is not set */
#define CONFIG_ENABLE_IOPAD
#define CONFIG_USE_SPI
#define CONFIG_USE_FSPIM
/* CONFIG_USE_QSPI is not set */
#define CONFIG_USE_SERIAL

/* Usart Configuration */

#define CONFIG_ENABLE_Pl011_UART
/* end of Usart Configuration */
#define CONFIG_USE_GPIO
/* CONFIG_ENABLE_FGPIO is not set */
/* CONFIG_USE_ETH is not set */
/* CONFIG_USE_CAN is not set */
/* CONFIG_USE_I2C is not set */
/* CONFIG_USE_TIMER is not set */
/* CONFIG_USE_MIO is not set */
/* CONFIG_USE_SDMMC is not set */
/* CONFIG_USE_PCIE is not set */
/* CONFIG_USE_WDT is not set */
#define CONFIG_USE_DMA
/* CONFIG_ENABLE_FGDMA is not set */
#define CONFIG_ENABLE_FDDMA
/* CONFIG_USE_NAND is not set */
/* CONFIG_USE_RTC is not set */
#define CONFIG_USE_SATA

/* FSATA Configuration */

#define CONFIG_ENABLE_FSATA
/* end of FSATA Configuration */
/* CONFIG_USE_USB is not set */
/* CONFIG_USE_ADC is not set */
/* CONFIG_USE_PWM is not set */
/* CONFIG_USE_IPC is not set */
/* CONFIG_USE_MEDIA is not set */
/* CONFIG_USE_SCMI_MHU is not set */
/* CONFIG_USE_I2S is not set */
/* end of Drivers configuration */

/* Third-party configuration */

/* CONFIG_USE_LWIP is not set */
#define CONFIG_USE_LETTER_SHELL

/* Letter shell configuration */

#define CONFIG_LS_PL011_UART
#define CONFIG_DEFAULT_LETTER_SHELL_USE_UART1
/* CONFIG_DEFAULT_LETTER_SHELL_USE_UART0 is not set */
/* CONFIG_DEFAULT_LETTER_SHELL_USE_UART2 is not set */
/* end of Letter shell configuration */
/* CONFIG_USE_AMP is not set */
/* CONFIG_USE_YMODEM is not set */
/* CONFIG_USE_SFUD is not set */
/* CONFIG_USE_FATFS_0_1_4 is not set */
#define CONFIG_USE_TLSF
/* CONFIG_USE_SPIFFS is not set */
/* CONFIG_USE_LITTLE_FS is not set */
/* CONFIG_USE_LVGL is not set */
/* CONFIG_USE_FREEMODBUS is not set */
/* CONFIG_USE_FSL_SDMMC is not set */
/* CONFIG_USE_MICROPYTHON is not set */
/* end of Third-party configuration */

/* Build setup */

#define CONFIG_CHECK_DEPS
#define CONFIG_OUTPUT_BINARY

/* Optimization options */

/* CONFIG_DEBUG_NOOPT is not set */
/* CONFIG_DEBUG_CUSTOMOPT is not set */
#define CONFIG_DEBUG_FULLOPT
#define CONFIG_DEBUG_OPT_UNUSED_SECTIONS
#define CONFIG_DEBUG_LINK_MAP
/* CONFIG_CCACHE is not set */
/* CONFIG_ARCH_COVERAGE is not set */
/* CONFIG_LTO_FULL is not set */
/* end of Optimization options */

/* Debug options */

/* CONFIG_DEBUG_ENABLE_ALL_WARNING is not set */
/* CONFIG_WALL_WARNING_ERROR is not set */
/* CONFIG_STRICT_PROTOTYPES is not set */
/* CONFIG_DEBUG_SYMBOLS is not set */
/* CONFIG_FRAME_POINTER is not set */
/* CONFIG_OUTPUT_ASM_DIS is not set */
/* CONFIG_ENABLE_WSHADOW is not set */
/* CONFIG_ENABLE_WUNDEF is not set */
#define CONFIG_DOWNGRADE_DIAG_WARNING
/* end of Debug options */

/* Lib */

#define CONFIG_USE_COMPILE_CHAIN
/* CONFIG_USE_NEWLIB is not set */
/* CONFIG_USE_USER_DEFINED is not set */
/* end of Lib */
/* CONFIG_ENABLE_CXX is not set */

/* Linker Options */

#define CONFIG_DEFAULT_LINKER_SCRIPT
/* CONFIG_USER_DEFINED_LD is not set */
#define CONFIG_IMAGE_LOAD_ADDRESS 0x80100000
#define CONFIG_IMAGE_MAX_LENGTH 0x1000000
#define CONFIG_HEAP_SIZE 1
#define CONFIG_SVC_STACK_SIZE 0x1000
#define CONFIG_SYS_STACK_SIZE 0x1000
#define CONFIG_IRQ_STACK_SIZE 0x1000
#define CONFIG_ABORT_STACK_SIZE 0x1000
#define CONFIG_FIQ_STACK_SIZE 0x1000
#define CONFIG_UNDEF_STACK_SIZE 0x1000
/* end of Linker Options */
/* end of Build setup */

#endif
