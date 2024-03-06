

# Add the unistd C files to the build


# The remaining sources files depend upon C streams

	
LIBM_CSRCS +=	\
	feclearexcept.c fe_dfl_env.c fegetenv.c fegetexceptflag.c \
	fegetround.c feholdexcept.c feraiseexcept.c fesetenv.c \
	fesetexceptflag.c fesetround.c fetestexcept.c feupdateenv.c