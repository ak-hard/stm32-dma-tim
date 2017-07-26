SECTIONS
{
    .isr_vector_core :
    {
       . = ALIGN(4);
        KEEP(*(.isr_vector_core))
       . = ALIGN(4);
    } >FLASH
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
       . = ALIGN(4);
    } >FLASH
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } >FLASH
    _sidata = LOADADDR(.data);
    .data :
    {
        . = ALIGN(4);
        _sdata = . ;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = . ;
    } >RAM AT>FLASH
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = . ;
    } >RAM
}
