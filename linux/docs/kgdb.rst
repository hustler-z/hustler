+--------------------------------------------------------------------------------------+
| Refer to Linux-6.1.63                                                                |
+--------------------------------------------------------------------------------------+

+-------------+                      +------------+
| remote host |_____ connection _____|   Target   |
|     GDB     |           |          |    KGDB    |
+-------------+           |          +------------+
                   serial/ethernet

----------------------------------------------------------------------------------------
[1] Configuration to enable/disable kdb

# CONFIG_STRICT_KERNEL_RWX is not set
CONFIG_FRAME_POINTER=y
CONFIG_KGDB=y
CONFIG_KGDB_SERIAL_CONSOLE=y
CONFIG_KGDB_KDB=y
CONFIG_KDB_KEYBOARD=y
CONFIG_MAGIC_SYSRQ=y
CONFIG_MAGIC_SYSRQ_SERIAL=y
CONFIG_DEBUG_INFO=y

CONFIG_RANDOMIZE_BASE=n

[2] kgdboc (kgdb over console)

For kgdb/gdb, kgdboc is designed to work with a single serial port. The Kernel command
line option kgdbwait makes kgdb wait for a debugger connection during booting of a 
kernel. The kgdbcon feature allows you to see printk() messages inside gdb while gdb
is connected to the kernel.

[3] kgdboe (kgdb over ethernet)
----------------------------------------------------------------------------------------
On AArch64 Linux:

$ lkvm run -k Image -d rootfs.ext4 -c 6 -m 512 -n disk --debug \
           --console serial -p "kgdboc=ttyS1 kgdbwait kgdbcon" --tty 1

  Info: Assigned terminal 1 to pty /dev/pts/[X]
  ...

[Tip: kvmtool does not work so far]

$ qemu-system-aarch64 \
    -machine virt \
    -cpu cortex-a53 \
    -smp 2 \
    -m 1G \
    -kernel Image.gz \
    -drive format=raw,index=0,file=rootfs.ext4 \
    -nographic \
    -append "noinitrd root=/dev/vda console=ttyAMA0 init=/linuxrc nokaslr" \
    -S -gdb tcp::5050

[Tip: qemu works properly so far]

$ gdb vmlinux
(gdb) target remote /dev/pts/[X]
(gdb) set debug-file-directory
(gdb) b [func]
(gdb) monitor ps
(gdb) i registers
(gdb) i all-registers
(gdb) i registers [reg]
(gdb) p $[reg]
(gdb) p/x $[reg]
(gdb) x $[reg]
(gdb) disassemble [func]
(gdb) bt
(gdb) c
(gdb) s
(gdb) l [func]
----------------------------------------------------------------------------------------
Enter the kernel debugger manually or by waiting for an oops or fault.
CONFIG_MAGIC_SYSRQ=y.

$ echo g > /proc/sysrq-trigger
----------------------------------------------------------------------------------------
