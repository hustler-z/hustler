

# Add the unistd C files to the build


# The remaining sources files depend upon C streams
	
LIBS_CSRCS +=	\
	asctime.c	\
	asctime_r.c	\
	clock.c		\
	ctime.c		\
	ctime_r.c	\
	difftime.c	\
	gettzinfo.c	\
	gmtime.c	\
	gmtime_r.c	\
	lcltime.c	\
	lcltime_r.c	\
	mktime.c	\
	month_lengths.c \
	strftime.c  	\
	strptime.c	\
	time.c		\
	tzcalc_limits.c \
	tzlock.c	\
	tzset.c		\
	tzset_r.c	\
	tzvars.c	\
	wcsftime.c



