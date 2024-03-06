
ifdef CONFIG_USE_LETTER_SHELL
$(BUILD_OUT_PATH)/lib_letter_shell.a: lib_letter_shell.a
lib_letter_shell.a: 
	$(call invoke_make_in_directory,third-party/letter-shell-3.1,makefile,all,)
lib_letter_shell_debug:
	$(call invoke_make_in_directory,third-party/letter-shell-3.1,makefile,debug,)
lib_letter_shell_info:
	$(call invoke_make_in_directory,third-party/letter-shell-3.1,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_letter_shell.a
$(BUILD_OUT_PATH)/lib_letter_shell.json:lib_letter_shell.json
lib_letter_shell.json:
	$(call invoke_make_in_directory,third-party/letter-shell-3.1,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_letter_shell.json
endif

ifdef CONFIG_USE_LWIP
$(BUILD_OUT_PATH)/liblwip.a: liblwip.a
liblwip.a: 
	$(call invoke_make_in_directory,third-party/lwip-2.1.2,makefile,all,)
liblwip_debug:
	$(call invoke_make_in_directory,third-party/lwip-2.1.2,makefile,debug,)
liblwip_info:
	$(call invoke_make_in_directory,third-party/lwip-2.1.2,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/liblwip.a
$(BUILD_OUT_PATH)/liblwip.json:liblwip.json
liblwip.json:
	$(call invoke_make_in_directory,third-party/lwip-2.1.2,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/liblwip.json
endif

ifdef CONFIG_USE_SFUD
$(BUILD_OUT_PATH)/libsfud.a: libsfud.a
libsfud.a: 
	$(call invoke_make_in_directory,third-party/sfud-1.1.0,makefile,all,)
libsfud_debug:
	$(call invoke_make_in_directory,third-party/sfud-1.1.0,makefile,debug,)
libsfud_info:
	$(call invoke_make_in_directory,third-party/sfud-1.1.0,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libsfud.a
$(BUILD_OUT_PATH)/libsfud.json:libsfud.json
libsfud.json:
	$(call invoke_make_in_directory,third-party/sfud-1.1.0,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/libsfud.json
endif #CONFIG_USE_SFUD

ifdef CONFIG_USE_TLSF
$(BUILD_OUT_PATH)/libtlsf.a: libtlsf.a
libtlsf.a:
	$(call invoke_make_in_directory,third-party/tlsf-3.1.0,makefile,all,)
libtlsf_debug:
	$(call invoke_make_in_directory,third-party/tlsf-3.1.0,makefile,debug,)
libtlsf_info:
	$(call invoke_make_in_directory,third-party/tlsf-3.1.0,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libtlsf.a
$(BUILD_OUT_PATH)/libtlsf.json:libtlsf.json
libtlsf.json:
	$(call invoke_make_in_directory,third-party/tlsf-3.1.0,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/libtlsf.json
endif

ifdef CONFIG_USE_SPIFFS
$(BUILD_OUT_PATH)/libspiffs.a: libspiffs.a
libspiffs.a:
	$(call invoke_make_in_directory,third-party/spiffs-0.3.7,makefile,all,)
spiffs_debug:
	$(call invoke_make_in_directory,third-party/spiffs-0.3.7,makefile,debug,)
spiffs_iinfo:
	$(call invoke_make_in_directory,third-party/spiffs-0.3.7,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libspiffs.a
$(BUILD_OUT_PATH)/libspiffs.json:libspiffs.json
libspiffs.json:
	$(call invoke_make_in_directory,third-party/spiffs-0.3.7,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/libspiffs.json
endif

# freemodbus-v1.6
ifdef CONFIG_USE_FREEMODBUS
$(BUILD_OUT_PATH)/libfreemodbus.a: libfreemodbus.a
libfreemodbus.a:
	$(call invoke_make_in_directory,third-party/freemodbus-v1.6,makefile,all,)
freemodbus_debug:
	$(call invoke_make_in_directory,third-party/freemodbus-v1.6,makefile,debug,)
freemodbus_info:
	$(call invoke_make_in_directory,third-party/freemodbus-v1.6,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libfreemodbus.a
$(BUILD_OUT_PATH)/libfreemodbus.json:libfreemodbus.json
libfreemodbus.json:
	$(call invoke_make_in_directory,third-party/freemodbus-v1.6,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/libfreemodbus.json
endif
# libmetal
ifdef CONFIG_USE_LIBMETAL
$(BUILD_OUT_PATH)/lib_libmetal.a: lib_libmetal.a
lib_libmetal.a:
	$(call invoke_make_in_directory,third-party/libmetal,makefile,all,)
lib_libmetal_debug:
	$(call invoke_make_in_directory,third-party/libmetal,makefile,debug,)
lib_libmetal_info:
	$(call invoke_make_in_directory,third-party/libmetal,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_libmetal.a
$(BUILD_OUT_PATH)/lib_libmetal.json:lib_libmetal.json
lib_libmetal.json:
	$(call invoke_make_in_directory,third-party/libmetal,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_libmetal.json
endif

# openamp
ifdef CONFIG_USE_OPENAMP
$(BUILD_OUT_PATH)/lib_openamp.a: lib_openamp.a
lib_openamp.a:
	$(call invoke_make_in_directory,third-party/openamp,makefile,all,)
lib_openamp_debug:
	$(call invoke_make_in_directory,third-party/openamp,makefile,debug,)
lib_openamp_info:
	$(call invoke_make_in_directory,third-party/openamp,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_openamp.a
$(BUILD_OUT_PATH)/lib_openamp.json:lib_openamp.json
lib_openamp.json:
	$(call invoke_make_in_directory,third-party/openamp,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_openamp.json
endif


# Crypto++
ifdef CONFIG_USE_CRYPTO_PLUS_PLUS
$(BUILD_OUT_PATH)/lib_crypto_pp.a: lib_crypto_pp.a
lib_crypto_pp.a:
	$(call invoke_make_in_directory,third-party/crypto++,makefile,all,)
lib_crypto_pp_debug:
	$(call invoke_make_in_directory,third-party/crypto++,makefile,debug,)
lib_crypto_pp_info:
	$(call invoke_make_in_directory,third-party/crypto++,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_crypto_pp.a
$(BUILD_OUT_PATH)/lib_crypto_pp.json:lib_crypto_pp.json
lib_crypto_pp.json:
	$(call invoke_make_in_directory,third-party/crypto++,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_crypto_pp.json
endif


ifdef CONFIG_USE_LVGL
$(BUILD_OUT_PATH)/lib_lv.a: lib_lv.a
lib_lv.a:
	$(call invoke_make_in_directory,third-party/lvgl-8.3,makefile,all,)
lib_lv_debug:
	$(call invoke_make_in_directory,third-party/lvgl-8.3,makefile,debug,)
lib_lv_info:
	$(call invoke_make_in_directory,third-party/lvgl-8.3,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_lv.a
$(BUILD_OUT_PATH)/lib_lv.json:lib_lv.json
lib_lv.json:
	$(call invoke_make_in_directory,third-party/lvgl-8.3,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_lv.json
endif

# FatFs_0_1_4
ifdef CONFIG_USE_FATFS_0_1_4
$(BUILD_OUT_PATH)/lib_fatfs.a: lib_fatfs.a
lib_fatfs.a:
	$(call invoke_make_in_directory,third-party/fatfs-0.1.4,makefile,all,)
lib_fatfs_debug:
	$(call invoke_make_in_directory,third-party/fatfs-0.1.4,makefile,debug,)
lib_fatfs_info:
	$(call invoke_make_in_directory,third-party/fatfs-0.1.4,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_fatfs.a
$(BUILD_OUT_PATH)/lib_fatfs.json:lib_fatfs.json
lib_fatfs.json:
	$(call invoke_make_in_directory,third-party/fatfs-0.1.4,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_fatfs.json
endif

# fsl sdmmc
ifdef CONFIG_USE_FSL_SDMMC
$(BUILD_OUT_PATH)/libfslsdmmc.a: libfslsdmmc.a
libfslsdmmc.a:
	$(call invoke_make_in_directory,third-party/fsl_sdmmc,makefile,all,)
libfslsdmmc_debug:
	$(call invoke_make_in_directory,third-party/fsl_sdmmc,makefile,debug,)
libfslsdmmc_info:
	$(call invoke_make_in_directory,third-party/fsl_sdmmc,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libfslsdmmc.a
$(BUILD_OUT_PATH)/libfslsdmmc.json:libfslsdmmc.json
libfslsdmmc.json:
	$(call invoke_make_in_directory,third-party/fsl_sdmmc,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/libfslsdmmc.json
endif

ifdef CONFIG_USE_MICROPYTHON
$(BUILD_OUT_PATH)/lib_mpy.a: lib_mpy.a
lib_mpy.a:
	$(call invoke_make_in_directory,third-party/micropython,makefile,all,)
lib_mpy_debug:
	$(call invoke_make_in_directory,third-party/micropython,makefile,debug,)
lib_mpy_info:
	$(call invoke_make_in_directory,third-party/micropython,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_mpy.a
$(BUILD_OUT_PATH)/lib_mpy.json:lib_mpy.json
lib_mpy.json:
	$(call invoke_make_in_directory,third-party/micropython,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_mpy.json
endif