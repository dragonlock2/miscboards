#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <chip.h>

extern uint8_t _data_rom;
extern uint8_t _data;
extern uint8_t _edata;
extern uint8_t _bss;
extern uint8_t _ebss;
extern uint8_t _eram;

extern void __libc_init_array(void);
extern void __libc_fini_array(void);
extern int main(void);

void _init(void) {}
void _fini(void) {}

void reset_handler(void);

static void default_handler(void) { while (1); }

__attribute__((used, section(".vectors")))
static void (*const rom_vectors[8])(void) = {
    (void(*)(void)) &_eram,
    reset_handler,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    (void(*)(void)) -(0x10008000 + 0x00000021), // hardcode checksum :P
};

static void (*ram_vectors[51])(void) __attribute__((aligned(1024)));

__attribute__((used, naked, optimize("O0"), section(".reset_handler")))
void reset_handler(void) {
    memcpy(&_data, &_data_rom, &_edata - &_data);
    memset(&_bss, 0, &_ebss - &_bss);

    ram_vectors[0] = (void(*)(void)) &_eram;
    ram_vectors[1] = reset_handler;
    for (size_t i = 2; i < sizeof(ram_vectors) / sizeof(ram_vectors[0]); i++) {
        ram_vectors[i] = default_handler;
    }
    SCB->VTOR = (uint32_t) ram_vectors;

    __libc_init_array();
    main();
    __libc_fini_array();
    while (1);
}
