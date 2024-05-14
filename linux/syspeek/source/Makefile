# ----------------------------------------------------------
# BPF PROGRAMS
# ----------------------------------------------------------

bpf-programs += memleak/
bpf-programs += irqs/
bpf-programs += irqoff/
bpf-programs += offcpu/
bpf-programs += rqlatency/
bpf-programs += wqlatency/

all:
	@for prog in $(bpf-programs); do \
		echo "- building $$prog -";  \
		make -C $$prog;              \
	done

# ----------------------------------------------------------
