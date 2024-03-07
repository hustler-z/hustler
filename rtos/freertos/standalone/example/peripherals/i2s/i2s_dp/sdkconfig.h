#ifndef SDK_CONFIG_H__
#define SDK_CONFIG_H__

#define CONFIG_USE_BAREMETAL

/* Arch configuration */

#define CONFIG_TARGET_ARMv8
#define CONFIG_ARCH_NAME "armv8"

/* Arm architecture configuration */

#define CONFIG_ARCH_ARMV8_AARCH64
/* CONFIG_ARCH_ARMV8_AARCH32 is not set */

/* Compiler configuration */

#define CONFIG_ARM_GCC_SELECT
/* CONFIG_ARM_CLANG_SELECT is not set */
#define CONFIG_TOOLCHAIN_NAME "gcc"
#define CONFIG_TARGET_ARMV8_AARCH64
#define CONFIG_ARCH_EXECUTION_STATE "aarch64"
#define CONFIG_ARM_NEON
#define CONFIG_ARM_CRC
#define CONFIG_ARM_CRYPTO
#define CONFIG_ARM_FLOAT_POINT
/* CONFIG_GCC_CODE_MODEL_TINY is not set */
#define CONFIG_GCC_CODE_MODEL_SMALL
/* CONFIG_GCC_CODE_MODEL_LARGE is not set */
/* end of Compiler configuration */
/* CONFIG_USE_CACHE is not set */
#define CONFIG_BOOT_WITH_FLUSH_CACHE
/* CONFIG_MMU_DEBUG_PRINTS is not set */
/* end of Arm architecture configuration */
/* end of Arch configuration */

/* Soc configuration */

/* CONFIG_TARGET_PHYTIUMPI is not set */
#define CONFIG_TARGET_E2000Q
/* CONFIG_TARGET_E2000D is not set */
/* CONFIG_TARGET_E2000S is not set */
/* CONFIG_TARGET_FT2004 is not set */
/* CONFIG_TARGET_D2000 is not set */
#define CONFIG_SOC_NAME "e2000"
#define CONFIG_TARGET_TYPE_NAME "q"
#define CONFIG_SOC_CORE_NUM 4
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

#define CONFIG_BOARD_NAME "demo"
/* CONFIG_USE_SPI_IOPAD is not set */
/* CONFIG_USE_GPIO_IOPAD is not set */
/* CONFIG_USE_CAN_IOPAD is not set */
/* CONFIG_USE_QSPI_IOPAD is not set */
/* CONFIG_USE_PWM_IOPAD is not set */
/* CONFIG_USE_MIO_IOPAD is not set */
/* CONFIG_USE_TACHO_IOPAD is not set */
/* CONFIG_USE_UART_IOPAD is not set */
/* CONFIG_USE_THIRD_PARTY_IOPAD is not set */
#define CONFIG_E2000Q_DEMO_BOARD

/* IO mux configuration when board start up */

/* end of IO mux configuration when board start up */
/* CONFIG_CUS_DEMO_BOARD is not set */

/* Build project name */

#define CONFIG_TARGET_NAME "i2s"
/* end of Build project name */
/* end of Board Configuration */

/* Sdk common configuration */

/* CONFIG_LOG_VERBOS is not set */
/* CONFIG_LOG_DEBUG is not set */
/* CONFIG_LOG_INFO is not set */
/* CONFIG_LOG_WARN is not set */
#define CONFIG_LOG_ERROR
/* CONFIG_LOG_NONE is not set */
/* CONFIG_LOG_EXTRA_INFO is not set */
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
/* CONFIG_USE_SPI is not set */
/* CONFIG_USE_QSPI is not set */
#define CONFIG_USE_SERIAL

/* Usart Configuration */

#define CONFIG_ENABLE_Pl011_UART
/* end of Usart Configuration */
/* CONFIG_USE_GPIO is not set */
/* CONFIG_USE_ETH is not set */
/* CONFIG_USE_CAN is not set */
#define CONFIG_USE_I2C
#define CONFIG_USE_FI2C
/* CONFIG_USE_TIMER is not set */
#define CONFIG_USE_MIO

/* Hardware Mio Configuration */

#define CONFIG_ENABLE_MIO
/* end of Hardware Mio Configuration */
/* CONFIG_USE_SDMMC is not set */
/* CONFIG_USE_PCIE is not set */
/* CONFIG_USE_WDT is not set */
#define CONFIG_USE_DMA
/* CONFIG_ENABLE_FGDMA is not set */
#define CONFIG_ENABLE_FDDMA
/* CONFIG_USE_NAND is not set */
/* CONFIG_USE_RTC is not set */
/* CONFIG_USE_SATA is not set */
/* CONFIG_USE_USB is not set */
/* CONFIG_USE_ADC is not set */
/* CONFIG_USE_PWM is not set */
/* CONFIG_USE_IPC is not set */
#define CONFIG_USE_MEDIA

/* Media Configuration */

#define CONFIG_ENABLE_FDC_DP_USE_LIB
/* end of Media Configuration */
/* CONFIG_USE_SCMI_MHU is not set */
#define CONFIG_USE_I2S

/* I2S Configuration */

#define CONFIG_USE_ES8336
#define CONFIG_USE_FI2S
/* end of I2S Configuration */
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
#define CONFIG_OUTPUT_ASM_DIS
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
#define CONFIG_IMAGE_MAX_LENGTH 0x10000000
#define CONFIG_HEAP_SIZE 1
#define CONFIG_STACK_SIZE 0x400
/* end of Linker Options */
/* end of Build setup */

#endif
