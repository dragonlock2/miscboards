MEMORY {
    ROM  (rx):  ORIGIN = 0x00000000, LENGTH = 512K
    RAM  (rwx): ORIGIN = 0x10000000, LENGTH = 32K
    RAM1 (rwx): ORIGIN = 0x2007C000, LENGTH = 16K
    RAM2 (rwx): ORIGIN = 0x20080000, LENGTH = 16K
}

SECTIONS {
    .text : ALIGN(4) {
        KEEP(*(.vectors));
        KEEP(*(.reset_handler));
        *(.text*);
        *(.rodata*);
        *(.srodata*);
    } > ROM

    .preinit_array (READONLY) : ALIGN(4) {
        __preinit_array_start = .;
        KEEP(*(.preinit_array*));
        __preinit_array_end = .;
    } > ROM

    .init_array (READONLY) : ALIGN(4) {
        __init_array_start = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)));
        KEEP(*(.init_array*));
        __init_array_end = .;
    } > ROM

    .fini_array (READONLY) : ALIGN(4) {
        __fini_array_start = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)));
        KEEP(*(.fini_array*));
        __fini_array_end = .;
    } > ROM

    .data : ALIGN(4) {
        _data = .;
        *(.data*);
        *(.sdata*);
        *(.sdata2*);
        _edata = .;
    } > RAM AT > ROM
    _data_rom = LOADADDR(.data);

    .bss (NOLOAD) : ALIGN(4) {
        _bss = .;
        *(.bss*);
        *(.sbss*);
        *(.sbss2*);
        *(COMMON*);
        _ebss = .;
        end = .;
    } > RAM

    _eram = ORIGIN(RAM) + LENGTH(RAM);
}
