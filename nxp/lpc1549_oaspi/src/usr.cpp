#include <array>
#include <FreeRTOS.h>
#include <chip.h>
#include "usr.h"

#ifdef CONFIG_ADIN1110
static constexpr uint8_t PHYINT_PIN = 12;
static constexpr uint8_t PHYRST_PIN = 7;
#elifdef CONFIG_NCN26010
static constexpr uint8_t PHYINT_PIN = 18;
static constexpr uint8_t PHYRST_PIN = 11;
#endif

namespace usr {

static struct {
    int_callback phyint_cb;
    void *phyint_arg;
} data;

static void phyint_handler(void) {
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0));
    volatile auto cb  = data.phyint_cb;
    volatile auto arg = data.phyint_arg;
    if (cb && arg) {
        cb(arg);
    }
}

__attribute__((constructor))
static void init(void) {
    Chip_GPIO_Init(LPC_GPIO);

    Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, PHYINT_PIN);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PHYINT_PIN, IOCON_MODE_PULLUP);
    Chip_INMUX_PinIntSel(0, 0, PHYINT_PIN);
    Chip_PININT_Init(LPC_GPIO_PIN_INT);
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0));
    Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(0));
    Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(0));
    NVIC_SetVector(PIN_INT0_IRQn, reinterpret_cast<uint32_t>(phyint_handler));
    NVIC_SetPriority(PIN_INT0_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8 - __NVIC_PRIO_BITS));
    NVIC_EnableIRQ(PIN_INT0_IRQn);

    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, PHYRST_PIN);

    rgb(0, 0, 0);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 24);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 25);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 26);

    Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 4);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4, IOCON_MODE_PULLUP);
}

uint8_t id(void) {
    std::array<uint32_t, 4> uid;
    Chip_IAP_ReadUID(&uid[0]);
    uint8_t id = 0xFF;
    for (int i = 0; i < 16; i++) {
        id ^= reinterpret_cast<uint8_t*>(&uid[0])[i];
    }
    return id;
}

void phyint(int_callback cb, void *arg) {
    data.phyint_cb  = cb;
    data.phyint_arg = arg;
}

void phyrst(bool assert) {
    Chip_GPIO_SetPinState(LPC_GPIO, 0, PHYRST_PIN, !assert);
}

void rgb(bool r, bool g, bool b) {
    Chip_GPIO_SetPinState(LPC_GPIO, 0, 24, !r);
    Chip_GPIO_SetPinState(LPC_GPIO, 0, 25, !b);
    Chip_GPIO_SetPinState(LPC_GPIO, 0, 26, !g);
}

bool btn(void) {
    return !Chip_GPIO_GetPinState(LPC_GPIO, 0, 4);
}

};
