# ----------------------------------------------------------
# SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
# ----------------------------------------------------------
# Hustler's Projects                          2024/05/15 WED
# ----------------------------------------------------------
# Build user-space code
# targets ...: target-pattern: prereq-patterns
#       recipe
#       ...
# The target-pattern and prereq-patterns say how to compute
# the prerequisites of each target. Each target is matched
# against the target-pattern to extract a part of the target
# name, call the stem.
# ----------------------------------------------------------
# Order-only prerequisites: ensure a prerequisites is built
# before a target, but without forcing the target to be
# updated if the prerequisite is updated.
# targets : normal-prerequisites | order-only-prerequisites
# ----------------------------------------------------------

OUTPUT 	               ?= out
CLANG                  ?= clang
APPS                   ?=
LLVM_STRIP             ?= llvm-strip
LIBBPF_SRC             := $(abspath ../../libbpf/src)
BPFTOOL_SRC            := $(abspath ../../bpftool/src)
LIBBPF_OBJ             := $(abspath $(OUTPUT)/libbpf.a)
BPFTOOL_OUTPUT         ?= $(abspath $(OUTPUT)/bpftool)
BPFTOOL                ?= $(BPFTOOL_OUTPUT)/bootstrap/bpftool
ARCH                   ?= $(shell uname -m | sed 's/x86_64/x86/' | sed 's/aarch64/arm64/')
VMLINUX                := ../../vmlinux/$(ARCH)/vmlinux.h
INCLUDES               := -I$(OUTPUT) -I../../libbpf/include/uapi -I$(dir $(VMLINUX)) $(EXTRA_INCLUDES)
CFLAGS                 := -g -Wall $(EXTRA_CFLAGS)
ALL_LDFLAGS            := $(LDFLAGS) $(EXTRA_LDFLAGS)
CLANG_BPF_SYS_INCLUDES := $(shell $(CLANG) -v -E - </dev/null 2>&1 \
	| sed -n '/<...> search starts here:/,/End of search list./{ s| \(/.*\)|-idirafter \1|p }')

ifeq ($(V),1)
	Q =
	msg =
else
	Q = @
	msg = @printf '  %-8s %s%s\n'					\
		"$(1)"					      	            \
		"$(patsubst $(abspath $(OUTPUT))/%,%,$(2))"	\
		"$(if $(3), $(3))";
	MAKEFLAGS += --no-print-directory
endif

.PHONY: all
all: $(APPS)

.PHONY: clean
clean:
	$(call msg,CLEAN)
	$(Q)rm -rf $(OUTPUT) $(APPS)

# ----------------------------------------------------------
# BUILDING PROCESS
# ----------------------------------------------------------
$(OUTPUT) $(OUTPUT)/libbpf $(BPFTOOL_OUTPUT):
	$(call msg,MKDIR,$@)
	$(Q)mkdir -p $@

# Build libbpf
$(LIBBPF_OBJ): $(wildcard $(LIBBPF_SRC)/*.[ch] $(LIBBPF_SRC)/Makefile) | $(OUTPUT)/libbpf
	$(call msg,LIB,$@)
	$(Q)$(MAKE) -C $(LIBBPF_SRC) BUILD_STATIC_ONLY=1	\
		OBJDIR=$(dir $@)/libbpf DESTDIR=$(dir $@)	    \
		INCLUDEDIR= LIBDIR= UAPIDIR=			        \
		install

# Build bpftool
$(BPFTOOL): | $(BPFTOOL_OUTPUT)
	$(call msg,BPFTOOL,$@)
	$(Q)$(MAKE) ARCH= CROSS_COMPILE= OUTPUT=$(BPFTOOL_OUTPUT)/ -C $(BPFTOOL_SRC)

# Build BPF code
$(OUTPUT)/%.bpf.o: %.bpf.c $(LIBBPF_OBJ) $(wildcard %.h) $(VMLINUX) | $(OUTPUT)
	$(call msg,BPF,$@)
	$(Q)$(CLANG) -g -O2 -target bpf -D__TARGET_ARCH_$(ARCH) $(INCLUDES) \
		$(CLANG_BPF_SYS_INCLUDES) -c $(filter %.c,$^) -o $@
	$(Q)$(LLVM_STRIP) -g $@ # strip useless DWARF info

# Generate BPF skeletons
$(OUTPUT)/%.skel.h: $(OUTPUT)/%.bpf.o | $(OUTPUT) $(BPFTOOL)
	$(call msg,GEN-SKEL,$@)
	$(Q)$(BPFTOOL) gen skeleton $< > $@

# Build user-space code
$(patsubst %,$(OUTPUT)/%.o,$(APPS)): %.o: %.skel.h

$(OUTPUT)/%.o: %.c $(wildcard %.h) | $(OUTPUT)
	$(call msg,CC,$@)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) -c $(filter %.c,$^) -o $@

# Build application binary
$(APPS): %: $(OUTPUT)/%.o $(LIBBPF_OBJ) | $(OUTPUT)
	$(call msg,BINARY,$@)
	$(Q)$(CC) $(CFLAGS) $^ $(ALL_LDFLAGS) -lelf -lz -o $@

# Delete failed targets
.DELETE_ON_ERROR:

# Keep intermediate (.skel.h, .bpf.o, etc) targets
.SECONDARY:
# ----------------------------------------------------------
