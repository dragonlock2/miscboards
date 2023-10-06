ENTRY(reset_handler)

MEMORY {
    ROM (rx):  ORIGIN = 0x00000000, LENGTH = 16K
    RAM (rxw): ORIGIN = 0x20000000, LENGTH = 2K
}

SECTIONS {
    .text : {
        KEEP(*(.reset_handler));
        *(.text*);
        *(.rodata*);
        *(.srodata*);
        . = ALIGN(4);
    } > ROM

    .data : {
        . = ALIGN(4);
        __global_pointer$ = . + 0x800;
        _data = .;
        *(.data*)
        *(.sdata*)
        *(.sdata2*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > ROM
    _data_rom = LOADADDR(.data);

    .bss (NOLOAD) : {
        . = ALIGN(4);
        _bss = .;
        *(.bss*)
        *(.sbss*)
        *(.sbss2*)
        *(COMMON*)
        . = ALIGN(4);
        _ebss = .;
        end = .;
    } > RAM

    _ram  = ORIGIN(RAM);
    _eram = ORIGIN(RAM) + LENGTH(RAM);
}
