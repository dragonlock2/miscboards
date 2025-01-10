#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>

extern "C" void app_main(void*) {
    printf("booted!\r\n");

    while (1) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
