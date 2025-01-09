#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include <pico/stdlib.h>

static void core_task(void*) {
    bool core = false;
    while (1) {
        core = !core;
        vTaskCoreAffinitySet(nullptr, core ? 0b01 : 0b10);
        printf("0x%08lx: core %d\r\n", xTaskGetTickCount(), portGET_CORE_ID());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

extern "C" void app_main(void*) {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, 1);
    printf("booted!\r\n");

    /*
     * On RISC-V, must be tskIDLE_PRIORITY or else never runs because core0 stuck in idle, at least
     * until next USB event. This is probably because some ISR is not yielding properly. Might be
     * explained by RISC-V port not supporting nested interrupts. Still, this sometimes gets stuck
     * with taskYIELD() not working...
     */
    xTaskCreate(core_task, "core_task", configMINIMAL_STACK_SIZE, nullptr, tskIDLE_PRIORITY, nullptr);

    while (1) {
        gpio_xor_mask(1 << PICO_DEFAULT_LED_PIN);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
