/**
 * vhustler linker script: vhustler.ld
 */

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH("aarch64")
ENTRY(vmm_start)

SECTIONS
{
	. = 0x40000000;

	.text ALIGN (0x1000):
	{
		*(vectors);
		*(.text);
	}
	.data ALIGN (0x1000):
	{
		*(.data)
	}
	.bss ALIGN (0x1000):
	{
		*(.bss COMMON)
	}
	.vm1 0x60000000:
	{
		*(.vm1)
	}
	. = 0x100000000;
	. = . + 0x400000;
	stack_top = .;
}
