ENTRY(reset_handler)

MEMORY {
    /* TODO cleanly support other variants */
    ROM (rx):  ORIGIN = 0x00000000, LENGTH = 64K
    RAM (rwx): ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS {
    .text : {
        KEEP(*(.reset_handler));
        *(.text*);
        *(.rodata*);
        *(.srodata*);
        . = ALIGN(4);
    } > ROM

    .preinit_array : {
        __preinit_array_start = .;
        KEEP(*(.preinit_array*));
        __preinit_array_end = .;
        . = ALIGN(4);
    } > ROM

    .init_array : {
        __init_array_start = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)));
        KEEP(*(.init_array*));
        __init_array_end = .;
        . = ALIGN(4);
    } > ROM

    .fini_array : {
        __fini_array_start = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)));
        KEEP(*(.fini_array*));
        __fini_array_end = .;
        . = ALIGN(4);
    } > ROM

    .data : {
        . = ALIGN(4);
        __global_pointer$ = . + 0x800;
        _data = .;
        *(.data*);
        *(.sdata*);
        *(.sdata2*);
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > ROM
    _data_rom = LOADADDR(.data);

    .bss (NOLOAD) : {
        . = ALIGN(4);
        _bss = .;
        *(.bss*);
        *(.sbss*);
        *(.sbss2*);
        *(COMMON*);
        . = ALIGN(4);
        _ebss = .;
        end = .;
    } > RAM

    _ram  = ORIGIN(RAM);
    _eram = ORIGIN(RAM) + LENGTH(RAM);
}
