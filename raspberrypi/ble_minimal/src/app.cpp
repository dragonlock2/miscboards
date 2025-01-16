#include <FreeRTOS.h>
#include <task.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>
#include "ble.h"

extern "C" void app_main(void*) {
    BLE ble(BLE::conn_type::PASSKEY_CONFIRM);
    printf("booted!\r\n");

    while (true) {
        printf("%d %ld\r\n", ble.connected(), ble.passkey().value_or(0));
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
