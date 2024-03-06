include $(PROJECT_DIR)/sdkconfig

include argz/src.mk
include ctype/src.mk
include errno/src.mk
include locale/src.mk
include misc/src.mk
include reent/src.mk
include search/src.mk
include signal/src.mk
include ssp/src.mk
include stdio/src.mk
include stdlib/src.mk
include string/src.mk
include time/src.mk

CSRCS_RELATIVE_FILES := $(foreach file, $(LIBS_CSRCS), $(wildcard argz/$(file)))
ASRCS_RELATIVE_FILES := $(foreach file, $(LIBS_CSRCS), $(wildcard argz/$(file)))

CSRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard ctype/$(file)))
ASRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard ctype/$(file)))

CSRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard errno/$(file)))
ASRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard errno/$(file)))

CSRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard locale/$(file)))
ASRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard locale/$(file)))

CSRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard misc/$(file)))
ASRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard misc/$(file)))

CSRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard reent/$(file)))
ASRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard reent/$(file)))

CSRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard search/$(file)))
ASRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard search/$(file)))

CSRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard signal/$(file)))
ASRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard signal/$(file)))

CSRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard ssp/$(file)))
ASRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard ssp/$(file)))

CSRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard stdio/$(file)))
ASRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard stdio/$(file)))

CSRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard stdlib/$(file)))
ASRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard stdlib/$(file)))

CSRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard string/$(file)))
ASRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard string/$(file)))

CSRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard time/$(file)))
ASRCS_RELATIVE_FILES += $(foreach file, $(LIBS_CSRCS), $(wildcard time/$(file)))


stdio/vfprintf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfprintf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfprintf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -c stdio/vfprintf.c -o  $(call CTARGET, stdio/vfprintf.c, $(BUILD_PATH)) 

stdio/vfiprintf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfprintf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfprintf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTEGER_ONLY -c stdio/vfprintf.c -o  $(call CTARGET, stdio/vfiprintf.c, $(BUILD_PATH)) 

stdio/svfprintf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfprintf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfprintf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DSTRING_ONLY -c stdio/vfprintf.c -o  $(call CTARGET, stdio/svfprintf.c, $(BUILD_PATH)) 

stdio/svfiprintf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfprintf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfprintf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTEGER_ONLY -DSTRING_ONLY -c stdio/vfprintf.c -o  $(call CTARGET, stdio/svfiprintf.c, $(BUILD_PATH)) 

USR_EXTRA_OBJS := stdio/vfprintf.o stdio/vfiprintf.o stdio/svfprintf.o stdio/svfiprintf.o
	
stdio/vfwprintf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwprintf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwprintf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -c stdio/vfwprintf.c -o  $(call CTARGET, stdio/vfwprintf.c, $(BUILD_PATH)) 

stdio/vfiwprintf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwprintf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwprintf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTEGER_ONLY -c stdio/vfwprintf.c -o  $(call CTARGET, stdio/vfiwprintf.c, $(BUILD_PATH)) 

stdio/svfwprintf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwprintf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwprintf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DSTRING_ONLY -c stdio/vfwprintf.c -o  $(call CTARGET, stdio/svfwprintf.c, $(BUILD_PATH)) 

stdio/svfiwprintf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwprintf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwprintf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTEGER_ONLY -DSTRING_ONLY -c stdio/vfwprintf.c -o  $(call CTARGET, stdio/svfiwprintf.c, $(BUILD_PATH)) 


USR_EXTRA_OBJS += stdio/vfwprintf.o stdio/vfiwprintf.o stdio/svfwprintf.o stdio/svfiwprintf.o



stdio/vfscanf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfscanf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfscanf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -c stdio/vfwprintf.c -o  $(call CTARGET, stdio/vfscanf.c, $(BUILD_PATH)) 

stdio/vfiscanf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfscanf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfscanf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTEGER_ONLY -c stdio/vfscanf.c -o  $(call CTARGET, stdio/vfiscanf.c, $(BUILD_PATH)) 

stdio/svfscanf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfscanf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfscanf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DSTRING_ONLY -c stdio/vfscanf.c -o  $(call CTARGET, stdio/svfscanf.c, $(BUILD_PATH)) 

stdio/svfiscanf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfscanf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfscanf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTEGER_ONLY -DSTRING_ONLY -c stdio/vfscanf.c -o  $(call CTARGET, stdio/svfiscanf.c, $(BUILD_PATH)) 

USR_EXTRA_OBJS += stdio/vfscanf.o stdio/vfiscanf.o stdio/svfscanf.o stdio/svfiscanf.o

stdio/vfwscanf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwscanf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwscanf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -c stdio/vfwscanf.c -o  $(call CTARGET, stdio/vfwscanf.c, $(BUILD_PATH)) 

stdio/vfiwscanf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwscanf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwscanf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTEGER_ONLY -c stdio/vfwscanf.c -o  $(call CTARGET, stdio/vfiwscanf.c, $(BUILD_PATH)) 

stdio/svfwscanf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwscanf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwscanf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DSTRING_ONLY -c stdio/vfwscanf.c -o  $(call CTARGET, stdio/svfwscanf.c, $(BUILD_PATH)) 

stdio/svfiwscanf.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwscanf.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdio/vfwscanf.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTEGER_ONLY -DSTRING_ONLY -c stdio/vfwscanf.c -o  $(call CTARGET, stdio/svfiwscanf.c, $(BUILD_PATH)) 

USR_EXTRA_OBJS += stdio/vfwscanf.o stdio/vfiwscanf.o stdio/svfwscanf.o stdio/svfiwscanf.o



stdlib/mallocr.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTERNAL_NEWLIB -DDEFINE_MALLOC -c stdlib/mallocr.c -o  $(call CTARGET, stdlib/mallocr.c, $(BUILD_PATH)) 

stdlib/freer.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTERNAL_NEWLIB -DDEFINE_FREE -c stdlib/mallocr.c -o  $(call CTARGET, stdlib/freer.c, $(BUILD_PATH)) 

stdlib/reallocr.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTERNAL_NEWLIB -DDEFINE_REALLOC -c stdlib/mallocr.c -o  $(call CTARGET, stdlib/reallocr.c, $(BUILD_PATH)) 

stdlib/callocr.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTERNAL_NEWLIB -DDEFINE_CALLOC -c stdlib/mallocr.c -o  $(call CTARGET, stdlib/callocr.c, $(BUILD_PATH)) 

stdlib/cfreer.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTERNAL_NEWLIB -DDEFINE_CFREE -c stdlib/mallocr.c -o  $(call CTARGET, stdlib/cfreer.c, $(BUILD_PATH)) 

stdlib/malignr.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTERNAL_NEWLIB -DDEFINE_MEMALIGN -c stdlib/mallocr.c -o  $(call CTARGET, stdlib/malignr.c, $(BUILD_PATH)) 

stdlib/vallocr.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTERNAL_NEWLIB -DDEFINE_VALLOC -c stdlib/mallocr.c -o  $(call CTARGET, stdlib/vallocr.c, $(BUILD_PATH))

stdlib/pvallocr.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTERNAL_NEWLIB -DDEFINE_PVALLOC -c stdlib/mallocr.c -o  $(call CTARGET, stdlib/pvallocr.c, $(BUILD_PATH))

stdlib/mallinfor.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTERNAL_NEWLIB -DDEFINE_MALLINFO -c stdlib/mallocr.c -o  $(call CTARGET, stdlib/mallinfor.c, $(BUILD_PATH))

stdlib/mallstatsr.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTERNAL_NEWLIB -DDEFINE_MALLOC_STATS -c stdlib/mallocr.c -o  $(call CTARGET, stdlib/mallstatsr.c, $(BUILD_PATH))

stdlib/msizer.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTERNAL_NEWLIB -DDEFINE_MALLOC_USABLE_SIZE -c stdlib/mallocr.c -o  $(call CTARGET, stdlib/msizer.c, $(BUILD_PATH))

stdlib/malloptr.o:
	$(Q) echo "  CC    $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c)) "
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname stdlib/mallocr.c))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -DINTERNAL_NEWLIB -DDEFINE_MALLOPT -c stdlib/mallocr.c -o  $(call CTARGET, stdlib/malloptr.c, $(BUILD_PATH))


USR_EXTRA_OBJS += stdlib/mallocr.o stdlib/freer.o stdlib/reallocr.o stdlib/callocr.o stdlib/cfreer.o stdlib/malignr.o stdlib/vallocr.o \
					stdlib/pvallocr.o stdlib/mallinfor.o stdlib/mallstatsr.o stdlib/msizer.o stdlib/malloptr.o 


#LIBS_NAME ?= libdrivers

include $(SDK_DIR)/tools/build/compiler.mk
