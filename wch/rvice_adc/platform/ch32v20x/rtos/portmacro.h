#include "RISC-V/portmacro.h"

#ifdef  portYIELD
#undef  portYIELD
#define portYIELD() NVIC_SetPendingIRQ(Software_IRQn);
#endif
