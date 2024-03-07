

# Add the unistd C files to the build


# The remaining sources files depend upon C streams

LIBS_CSRCS += \
	psignal.c \
	raise.c \
	signal.c
	