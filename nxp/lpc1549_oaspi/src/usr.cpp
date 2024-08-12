#include <chip.h>
#include "usr.h"

__attribute__((constructor))
static void usr_init(void) {
    Chip_GPIO_Init(LPC_GPIO);

    usr_rgb(0, 0, 0);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 24);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 25);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 26);

    Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 4);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4, IOCON_MODE_PULLUP);
}

void usr_rgb(bool r, bool g, bool b) {
    Chip_GPIO_SetPinState(LPC_GPIO, 0, 24, !r);
    Chip_GPIO_SetPinState(LPC_GPIO, 0, 25, !b);
    Chip_GPIO_SetPinState(LPC_GPIO, 0, 26, !g);
}

bool usr_btn(void) {
    return !Chip_GPIO_GetPinState(LPC_GPIO, 0, 4);
}
