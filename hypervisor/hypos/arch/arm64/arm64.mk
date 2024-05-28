# ----------------------------------------------------------
# Hustler's Project
#
# File:  arm64.mk
# Date:  2024/05/24
# Usage:
# ----------------------------------------------------------

arm64-asm-y += arch_boot.o
arm64-asm-y += arch_entry.o
arm64-asm-y += arch_mmu.o
arm64-asm-y += arch_debug.o

arm64-cc-y  += arch_ttbl.o
arm64-cc-y  += arch_excep.o
