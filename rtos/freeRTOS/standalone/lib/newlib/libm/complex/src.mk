

# Add the unistd C files to the build


# The remaining sources files depend upon C streams

	
LIBM_CSRCS +=	\
	cabs.c cacos.c cacosh.c carg.c casin.c casinh.c \
      catan.c catanh.c ccos.c ccosh.c cephes_subr.c \
      cexp.c cimag.c  clog.c clog10.c conj.c  \
      cpow.c cproj.c  creal.c  \
      csin.c csinh.c csqrt.c ctan.c ctanh.c	\
	cabsl.c creall.c cimagl.c ccoshl.c cacoshl.c \
       clogl.c csqrtl.c cargl.c cprojl.c cexpl.c \
       cephes_subrl.c cacosl.c ccosl.c casinl.c \
       catanhl.c conjl.c cpowl.c ctanhl.c ctanl.c \
       casinhl.c csinhl.c csinl.c catanl.c	\
	cabsf.c casinf.c ccosf.c cimagf.c cprojf.c  \
        csqrtf.c cacosf.c casinhf.c ccoshf.c clogf.c clog10f.c \
        crealf.c ctanf.c cacoshf.c catanf.c   \
        cephes_subrf.c conjf.c csinf.c ctanhf.c \
        cargf.c catanhf.c cexpf.c cpowf.c csinhf.c