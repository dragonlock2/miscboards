#include <chip.h>
#include "usr.h"

__attribute__((constructor))
static void usr_init(void) {
    Chip_GPIO_Init(LPC_GPIO);

    usr_rgb(0, 0, 0);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 3, 26);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 18);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 3, 25);

    Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 28);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 28, IOCON_MODE_PULLUP | IOCON_FUNC0); // P0.28 lacks pulls :(
}

void usr_rgb(bool r, bool g, bool b) {
    Chip_GPIO_SetPinState(LPC_GPIO, 3, 26, !r);
    Chip_GPIO_SetPinState(LPC_GPIO, 1, 18, !g);
    Chip_GPIO_SetPinState(LPC_GPIO, 3, 25, !b);
}

bool usr_btn(void) {
    return !Chip_GPIO_GetPinState(LPC_GPIO, 0, 28);
}
