#include <FreeRTOS.h>
#include <task.h>
#include <pico/stdlib.h>
#include <pico/sleep.h>

/* 
 * Pico: 600-650uA
 * - removed R5 and R10
 * 
 * Pico W: 550-600uA
 * - removed R5 and R10
 * 
 * Pico 2: 300uA
 * - removed R5 and R10
 */

static constexpr uint WAKE_PIN = 17;

#ifdef RASPBERRYPI_PICO

static void board_init(void) {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, 1);
}

static void led_set(bool en) {
    gpio_put(PICO_DEFAULT_LED_PIN, en);
}

static void sleep(void) {
    sleep_run_from_xosc();
    sleep_goto_dormant_until_pin(WAKE_PIN, true, false);
    sleep_power_up();
}

#elifdef RASPBERRYPI_PICO2

extern "C" {
#include <hardware/powman.h>
}

static void board_init(void) {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, 1);
}

static void led_set(bool en) {
    gpio_put(PICO_DEFAULT_LED_PIN, en);
}

static void sleep(void) {
    powman_enable_gpio_wakeup(0, WAKE_PIN, true, false);
    auto s = powman_get_power_state();
    // s = powman_power_state_with_domain_off(s, POWMAN_POWER_DOMAIN_SRAM_BANK1);
    // s = powman_power_state_with_domain_off(s, POWMAN_POWER_DOMAIN_XIP_CACHE);
    s = powman_power_state_with_domain_off(s, POWMAN_POWER_DOMAIN_SWITCHED_CORE);
    configASSERT(powman_set_power_state(s) == PICO_OK);
    __wfi();
}

#elifdef RASPBERRYPI_PICO_W

#include <pico/cyw43_arch.h>

static void board_init(void) {}

static void led_set(bool en) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, en);
}

static void sleep(void) {
    cyw43_arch_deinit(); // powers down cyw43
    sleep_run_from_xosc();
    sleep_goto_dormant_until_pin(WAKE_PIN, true, false);
    sleep_power_up();
    cyw43_arch_init(); // need reinit everything
}

#else
#error "unknown board"
#endif

void app_main(void*) {
    board_init();
    gpio_init(WAKE_PIN);
    gpio_pull_up(WAKE_PIN);

    while (1) {
        printf("0x%08lx: awoken\r\n", xTaskGetTickCount());
        uart_default_tx_wait_blocking();
        led_set(true);
        vTaskDelay(pdMS_TO_TICKS(1000));

        printf("0x%08lx: sleeping until GPIO%d falling edge\r\n", xTaskGetTickCount(), WAKE_PIN);
        uart_default_tx_wait_blocking();
        led_set(false);

        sleep();
    }
}
