# ----------------------------------------------------------
# Hustler's Project
#
# Date:  2024/07/19
# Usage: passthrough
#
# XXX: Device passthrough is about providing an isolation
#      of devices to a given guest operating system so that
#      device can be used exclusively by that guest.
#      (a) near-native performance.
#      (b) exclusively used by give guest.
# ----------------------------------------------------------

passthrough-y += iommu.o
passthrough-y += pci.o

# ----------------------------------------------------------
