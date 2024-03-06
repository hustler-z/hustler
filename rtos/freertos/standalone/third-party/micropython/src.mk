ifdef CONFIG_USE_MICROPYTHON	
	BACKTRACE_CSRCS += shared/readline/readline.c  
	BACKTRACE_CSRCS += shared/runtime/pyexec.c 
	BACKTRACE_CSRCS += shared/runtime/stdout_helpers.c 
	BACKTRACE_CSRCS += shared/libc/printf.c 
	BACKTRACE_CSRCS += shared/libc/abort_.c 
	BACKTRACE_CSRCS += shared/timeutils/timeutils.c 
	BACKTRACE_CSRCS += shared/runtime/interrupt_char.c 
	BACKTRACE_CSRCS += shared/libc/string0.c 	
	CSRCS_RELATIVE_FILES += $(wildcard ports/*.c) \
				+= $(wildcard py/*.c) \
				+= $(wildcard ports/genhdr/*.c) \
				+= $(wildcard ports/user/*.c) \
				+= $(wildcard extmod/*.c) \
				+= $(wildcard lib/oofatfs/*.c) \
				+= $(wildcard lib/libm/*.c) 
				
    CSRCS_RELATIVE_FILES += $(BACKTRACE_CSRCS)
endif

