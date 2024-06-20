# ----------------------------------------------------------
# Hustler's Project
#
# File:  config.mk
# Date:  2024/06/17
# Usage: Configuration of HYPOS
# ----------------------------------------------------------
define sed-offsets
	's:^[[:space:]]*\.ascii[[:space:]]*"\(.*\)".*:\1:; \
	/^->/{s:->#\(.*\):/* \1 */:; \
	s:^->\([^ ]*\) [\$$#]*\([^ ]*\) \(.*\):#define \1 \2 /* \3 */:; \
	s:->::; p;}'
endef

define asm_offsets
	$(Q)echo "GEN      $@"; \
	(set -e; \
	 echo "/*"; \
	 echo " * Hustler's Project"; \
	 echo " *"; \
	 echo " * File:  offset.h"; \
	 echo " * Date:"; \
	 echo " * Usage:"; \
	 echo " */"; \
	 echo ""; \
	 echo "#ifndef _ARCH_OFFSET_H"; \
	 echo "#define _ARCH_OFFSET_H"; \
	 echo ""; \
	 sed -ne $(sed-offsets) < $<; \
	 echo ""; \
	 echo "#endif /* _ARCH_OFFSET_H */") > $@
endef
# ----------------------------------------------------------

CFG_FRAME_POINTER := y

# ----------------------------------------------------------
