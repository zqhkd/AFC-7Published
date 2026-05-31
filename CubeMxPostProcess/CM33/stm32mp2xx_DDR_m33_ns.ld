/*
******************************************************************************
** Optimized Linker Script for MYD-LD25X (Cortex-M33 DDR Mode)
******************************************************************************
*/

ENTRY(Reset_Handler)

/* 栈顶位置：位于 8MB 数据区的末尾 */
_estack = 0x81200000; 

/* 🚨 提升堆栈安全边际：OpenAMP 需要更大的上下文空间 [cite: 648] 🚨 */
_Min_Heap_Size = 0x4000;  /* 16KB */
_Min_Stack_Size = 0x2000; /* 8KB */

/* 修复后的内存定义 */
MEMORY
{
  /* 🚨 1. 向量表：扩大到 2KB (0x800)，确保能装下 1344 字节 🚨 */
  NS_VECTOR_TBL (xrw) : ORIGIN = 0x80100000, LENGTH = 2K
  
  /* 🚨 2. FLASH：起始地址从 0x80100400 后移到 0x80100800，避开向量表 🚨 */
  FLASH         (rx)  : ORIGIN = 0x80100800, LENGTH = 8M - 2K
  
  /* 数据区 (cm33_cube_data): 0x80A00000, 8MB */
  RAM           (rwx) : ORIGIN = 0x80A00000, LENGTH = 8M
  
  /* 共享内存区 (按照阶段 1 基线) */
  VIRTIO_SHMEM  (rw)  : ORIGIN = 0x812F8000, LENGTH = 32K
}

SECTIONS
{
/* 中断向量表必须在 0x80100000 */
  .isr_vectors :
  {
    . = ALIGN(8);
    __Vectors = .;         /* 🚨 核心修复：向外导出 __Vectors 符号，它的值就是当前地址 0x80100000 🚨 */
    KEEP(*(.isr_vectors))
    . = ALIGN(8);
  } >NS_VECTOR_TBL

  /* 资源表：Linux 加载器的账本 [cite: 2755, 2834] */
  .resource_table :
  {
    . = ALIGN(8);
    KEEP(*(.resource_table))
    . = ALIGN(8);
  } >FLASH

  /* 合并后的程序代码段 */
  .text :
  {
    . = ALIGN(8);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(8);
  } >FLASH

  .rodata :
  {
    . = ALIGN(8);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(8);
  } >FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  _sidata = LOADADDR(.data);

  /* 数据段与堆栈全部放入 RAM 区 */
  .data :
  {
    . = ALIGN(8);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(8);
    _edata = .;
  } >RAM

  .bss :
  {
    . = ALIGN(8);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(8);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM

  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /* 明确标记 OpenAMP 区域 [cite: 2844] */
  __OPENAMP_region_start__ = ORIGIN(VIRTIO_SHMEM);
  __OPENAMP_region_end__   = ORIGIN(VIRTIO_SHMEM) + LENGTH(VIRTIO_SHMEM);

  /DISCARD/ : {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}