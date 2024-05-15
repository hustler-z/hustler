# ----------------------------------------------------------
# Hustler's Projects                          2024/05/15 WED
# ----------------------------------------------------------
JOBS           := $(shell nproc)
ARCH           ?= $(shell uname -m | sed 's/arm.*/arm/' \
					| sed 's/x86_64/x86/'               \
					| sed 's/riscv64/riscv/'            \
					| sed 's/aarch64/arm64/')

define allow-override
  	$(if $(or $(findstring environment,$(origin $(1))),\
            $(findstring command line,$(origin $(1)))),,\
    $(eval $(1) = $(2)))
endef

$(call allow-override,CC,$(CROSS_COMPILE)cc)
$(call allow-override,LD,$(CROSS_COMPILE)ld)
# ----------------------------------------------------------
# BPF Programs
# ----------------------------------------------------------

# bpf-programs += irqstat
bpf-programs += bootstrap
bpf-programs += wqlat
bpf-programs += schedhang

.PHONY: all
all:
	@for prog in $(bpf-programs); do  \
		echo "- Building $$prog -";   \
		make -j$(JOBS) -C $$prog;     \
	done

.PHONY: clean
clean:
	@for prog in $(bpf-programs); do  \
		echo "- Cleaning $$prog -";   \
		make -C $$prog clean;         \
	done
# ----------------------------------------------------------
