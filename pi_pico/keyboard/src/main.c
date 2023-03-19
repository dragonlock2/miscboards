#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>

int main() {
    cyw43_arch_init();
    stdio_init_all();

    int8_t c;
    while (true) {
        printf("hi %d\r\n", c++);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(500);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(500);
    }
}
