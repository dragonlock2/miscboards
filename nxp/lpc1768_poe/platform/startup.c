#include <stddef.h>
#include <stdint.h>
#include <string.h>

extern uint8_t _data_rom;
extern uint8_t _data;
extern uint8_t _edata;
extern uint8_t _bss;
extern uint8_t _ebss;
extern uint8_t _eram;

extern int main(void);

static void reset_handler(void);

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

__attribute__((naked, optimize("O0"), section(".reset_handler"))) // TODO max opt level?
static void reset_handler(void) {
    memcpy(&_data, &_data_rom, &_edata - &_data);
    memset(&_bss, 0, &_ebss - &_bss);

    // TODO move vector table, set all to default_handler
    // TODO libc funcs

    main();
    while (1);
}
