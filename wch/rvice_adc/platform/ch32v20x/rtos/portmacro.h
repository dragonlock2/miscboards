#include "RISC-V/portmacro.h"

#undef  portYIELD
#define portYIELD() NVIC_SetPendingIRQ(Software_IRQn);

#undef  portEND_SWITCHING_ISR
#define portEND_SWITCHING_ISR(xSwitchRequired) do { if (xSwitchRequired != pdFALSE) { portYIELD(); } } while (0)
