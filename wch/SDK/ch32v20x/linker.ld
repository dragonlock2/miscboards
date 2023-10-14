ENTRY(reset_handler)

MEMORY {
    /* TODO cleanly support other variants */
    ROM (rx):  ORIGIN = 0x00000000, LENGTH = 64K
    RAM (rwx): ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS {
    .text : ALIGN(4) {
        KEEP(*(.reset_handler));
        *(.text*);
        *(.rodata*);
        *(.srodata*);
        *(.gcc_except_table*);
    } > ROM

    .eh_frame : ALIGN(4) {
        KEEP(*(.eh_frame*));
    } > ROM

    .preinit_array : ALIGN(4) {
        __preinit_array_start = .;
        KEEP(*(.preinit_array*));
        __preinit_array_end = .;
    } > ROM

    .init_array : ALIGN(4) {
        __init_array_start = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)));
        KEEP(*(.init_array*));
        __init_array_end = .;
    } > ROM

    .fini_array : ALIGN(4) {
        __fini_array_start = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)));
        KEEP(*(.fini_array*));
        __fini_array_end = .;
    } > ROM

    .data : ALIGN(4) {
        __global_pointer$ = . + 0x800;
        _data = .;
        *(.data*);
        *(.sdata*);
        *(.sdata2*);
        _edata = .;
    } > RAM AT > ROM
    _data_rom = LOADADDR(.data);

    .bss (NOLOAD) : ALIGN(4) {
        __bss_start = .;
        _bss = .;
        *(.bss*);
        *(.sbss*);
        *(.sbss2*);
        *(COMMON*);
        _ebss = .;
        _end = .;
        end = .;
    } > RAM

    _ram  = ORIGIN(RAM);
    _eram = ORIGIN(RAM) + LENGTH(RAM);
}
