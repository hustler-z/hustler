

# Add the unistd C files to the build


# The remaining sources files depend upon C streams
	
LIBS_CSRCS +=	\
	chk_fail.c \
	stack_protector.c	\
	memcpy_chk.c \
	memmove_chk.c \
	mempcpy_chk.c \
	memset_chk.c \
	stpcpy_chk.c \
	stpncpy_chk.c \
	strcat_chk.c \
	strcpy_chk.c \
	strncat_chk.c \
	strncpy_chk.c	\
	gets_chk.c \
	snprintf_chk.c \
	sprintf_chk.c \
	vsnprintf_chk.c \
	vsprintf_chk.c



