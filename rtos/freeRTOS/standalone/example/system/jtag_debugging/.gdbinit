# increase the default remote timeout in gdb because 
#   Windows libusb transactions could be slow
set remotetimeout 100000

# gdb connect to openocd in port 3333 
target extended-remote localhost:3333

# start openocd working queue, monitor followed with openocd command here
monitor init

# force to use hardware breakpoint, otherwise use software breakpoint
#   for e2000d/q, num of hardware breakpoints supposed to be 6 
monitor gdb_breakpoint_override hardware

# load elf image
load ./baremetal.elf

# in case symbols skip load，load agin 
file ./baremetal.elf

# we can break at the beginning of code by address or by symbol
#break _boot

# add more breakpoints in application
#break JtagTouchRegisters
#break JtagTouchMemory
break bubbleSort
break JtagPostSort
#break bubbleSortCXX

# show all breakspoints we before running
info breakpoints

# show [-0x10 ~ +0x10 ] range of instructions when breaked
display /10i $pc-16

# start running
continue
