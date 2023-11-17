#include <stdint.h>
#include <string.h>

extern uint8_t _data_rom;
extern uint8_t _data;
extern uint8_t _edata;
extern uint8_t _bss;
extern uint8_t _ebss;
extern uint8_t _eram;

extern void __libc_init_array(void);
extern void __libc_fini_array(void);
extern int main(void);

__attribute__((interrupt)) static void default_handler(void) { while (1); }

void (*vectors[])(void) __attribute__((used)) = {
    default_handler,
    default_handler,
    default_handler, // NonMaskableInt_IRQn
    default_handler, // EXC_IRQn
    default_handler,
    default_handler, // Ecall_M_Mode_IRQn
    default_handler,
    default_handler,
    default_handler, // Ecall_U_Mode_IRQn
    default_handler, // Break_Point_IRQn
    default_handler,
    default_handler,
    default_handler, // SysTicK_IRQn
    default_handler,
    default_handler, // Software_IRQn
    default_handler,
    default_handler, // WWDG_IRQn
    default_handler, // PVD_IRQn
    default_handler, // TAMPER_IRQn
    default_handler, // RTC_IRQn
    default_handler, // FLASH_IRQn
    default_handler, // RCC_IRQn
    default_handler, // EXTI0_IRQn
    default_handler, // EXTI1_IRQn
    default_handler, // EXTI2_IRQn
    default_handler, // EXTI3_IRQn
    default_handler, // EXTI4_IRQn
    default_handler, // DMA1_Channel1_IRQn
    default_handler, // DMA1_Channel2_IRQn
    default_handler, // DMA1_Channel3_IRQn
    default_handler, // DMA1_Channel4_IRQn
    default_handler, // DMA1_Channel5_IRQn
    default_handler, // DMA1_Channel6_IRQn
    default_handler, // DMA1_Channel7_IRQn
    default_handler, // ADC_IRQn
    default_handler, // USB_HP_CAN1_TX_IRQn
    default_handler, // USB_LP_CAN1_RX0_IRQn
    default_handler, // CAN1_RX1_IRQn
    default_handler, // CAN1_SCE_IRQn
    default_handler, // EXTI9_5_IRQn
    default_handler, // TIM1_BRK_IRQn
    default_handler, // TIM1_UP_IRQn
    default_handler, // TIM1_TRG_COM_IRQn
    default_handler, // TIM1_CC_IRQn
    default_handler, // TIM2_IRQn
    default_handler, // TIM3_IRQn
    default_handler, // TIM4_IRQn
    default_handler, // I2C1_EV_IRQn
    default_handler, // I2C1_ER_IRQn
    default_handler, // I2C2_EV_IRQn
    default_handler, // I2C2_ER_IRQn
    default_handler, // SPI1_IRQn
    default_handler, // SPI2_IRQn
    default_handler, // USART1_IRQn
    default_handler, // USART2_IRQn
    default_handler, // USART3_IRQn
    default_handler, // EXTI15_10_IRQn
    default_handler, // RTCAlarm_IRQn
    default_handler, // USBWakeUp_IRQn
    default_handler, // USBHD_IRQn
    default_handler, // USBHDWakeUp_IRQn
#if defined(CH32V20x_D6)
    default_handler, // UART4_IRQn
    default_handler, // DMA1_Channel8_IRQn
#elif defined(CH32V20x_D8)
    default_handler, // ETH_IRQn
    default_handler, // ETHWakeUp_IRQn
    default_handler, // TIM5_IRQn
    default_handler, // UART4_IRQn
    default_handler, // DMA1_Channel8_IRQn
    default_handler, // OSC32KCal_IRQn
    default_handler, // OSCWakeUp_IRQn
#elif defined(CH32V20x_D8W)
    default_handler, // ETH_IRQn
    default_handler, // ETHWakeUp_IRQn
    default_handler, // BB_IRQn
    default_handler, // LLE_IRQn
    default_handler, // TIM5_IRQn
    default_handler, // UART4_IRQn
    default_handler, // DMA1_Channel8_IRQn
    default_handler, // OSC32KCal_IRQn
    default_handler, // OSCWakeUp_IRQn
#endif
};

__attribute__((used, naked, optimize("O1"), section(".reset_handler")))
void reset_handler(void) {
    asm volatile (
        ".option push             \n"
        ".option norelax          \n"
        "la gp, __global_pointer$ \n" // set gp
        ".option pop              \n"
        : : : "gp"
    );
    asm volatile ("mv sp, %0" : : "r" (&_eram) : ); // set sp

    memcpy(&_data, &_data_rom, &_edata - &_data); // setup data
    memset(&_bss, 0, &_ebss - &_bss); // setup bss

    asm volatile (
        "li t0, 0x1f       \n"
        "csrw 0xbc0, t0    \n" // from startup, exact effect unknown
        "li t0, 0x88       \n"
        "csrw mstatus, t0  \n" // enable MPIE/MIE
        "li t0, 0x3        \n"
        "csrw 0x804, t0    \n" // enable interrupt nesting, hardware stacking (INTSYSCR)
        "mv t0, %0         \n"
        "li t1, 0xfffffffc \n"
        "and t0, t0, t1    \n"
        "ori t0, t0, 0x3   \n"
        "csrw mtvec, t0    \n" // set vector table to point to RAM, use absolute addresses
        : : "r" (vectors) : "t0", "t1"
    );

    __libc_init_array();
    main();
    __libc_fini_array();

    while (1);
}
