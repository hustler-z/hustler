OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH("aarch64")
ENTRY(_start)

SECTIONS
{
 . = 0x10000000;

 . = ALIGN(0x1000);

 PROVIDE(_code_start = .);



 .text :
  {
  PROVIDE(_text_start = .);
  *(.entry)
  *(.text)
  . = ALIGN(8);
  PROVIDE(_text_end = .);
 }

 . = ALIGN(0x1000);

 .init :
 {
  PROVIDE(_init_start = .);

  PROVIDE(_init_text_start = .);

  *(.init.text)
  . = ALIGN(8);

  PROVIDE(__setup_start = .);
  *(.setup.init);
  PROVIDE(__setup_end = .);

  . = ALIGN(0x1000);

  PROVIDE(_init_text_end = .);

  PROVIDE(_initdata_start = .);
  *(.init.data)
  . = ALIGN(8);
  PROVIDE(_initdata_end = .);

  PROVIDE(_initconst_start = .);
  *(.init.rodata)
  . = ALIGN(8);
  PROVIDE(_initconst_end = .);

  PROVIDE(_init_end = .);
 }

 . = ALIGN(0x1000);

 .cpuinit :
 {
  PROVIDE(_cpuinit_start = .);
  *(.cpuinit.*)
  . = ALIGN(8);
  PROVIDE(_cpuinit_end = .);
 }

 . = ALIGN(0x1000);

 .spinlock :
 {
  PROVIDE(_spinlock_start = .);
  *(.spinlock.*)
  . = ALIGN(8);
  PROVIDE(_spinlock_end = .);
 }





 . = ALIGN(0x1000);

 .rodata :
 {
  PROVIDE(_rodata_start = .);
  *(.rodata .rodata.*)
  . = ALIGN(8);
  PROVIDE(_rodata_end = .);
 }





 . = ALIGN(0x1000);

 .percpu :
 {
  PROVIDE(_percpu_start = .);
  *(.percpu)
  . = ALIGN(8);
  PROVIDE(_percpu_end = .);
 }

 . = ALIGN(0x1000);

 .data :
 {
  PROVIDE(_data_start = .);

  *(.data)
  *(.data.*)
  *(.readmostly.data)
  *(*.data)
  . = ALIGN(8);

  PROVIDE(_modtbl_start = .);
  *(.modtbl)
  . = ALIGN(8);
  PROVIDE(_modtbl_end = .);

  PROVIDE(_nidtbl_start = .);
  *(.nidtbl)
  . = ALIGN(8);
  PROVIDE(_nidtbl_end = .);

  PROVIDE(_data_end = .);
 }

 . = ALIGN(0x1000);

 .bss :
 {
  PROVIDE(_bss_start = .);
  *(.bss)
  *(.bss.*)
  . = ALIGN(8);
  PROVIDE(_bss_end = .);
 }

 . = ALIGN(0x1000);

 .hvc_stack :
 {
  PROVIDE(_hvc_stack_start = .);
  . = . + (4096 * 8);
  . = ALIGN(8);
  PROVIDE(_hvc_stack_end = .);
 }



 . = ALIGN(0x1000);

 PROVIDE(_code_end = .);
}
