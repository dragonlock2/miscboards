#include <string>
#include <FreeRTOS.h>
#include <task.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>
#include "ble.h"

using namespace tinyusb;

extern "C" void app_main(void*) {
    BLE ble(BLE::conn_type::PASSKEY_CONFIRM);
    printf("booted!\r\n");

    bool prev_conn = false;
    while (true) {
        // demo on connnection
        bool conn = ble.connected();
        if (!prev_conn && conn) {
            // wait settle, may be too short right after pairing
            // if doesn't work still, may need to re-pair
            vTaskDelay(pdMS_TO_TICKS(1000));

            // keyboard
            printf("keyboard demo\r\n");
            static const uint8_t ASCII_TO_CODE[128][2] = { HID_ASCII_TO_KEYCODE };
            std::string s = "Hello World!";
            for (char &c: s) {
                hid_keyboard_report_t report {};
                report.keycode[0] = ASCII_TO_CODE[static_cast<uint8_t>(c)][1];
                report.modifier = ASCII_TO_CODE[static_cast<uint8_t>(c)][0] ? KEYBOARD_MODIFIER_LEFTSHIFT : 0;
                ble.send(report);
                report.keycode[0] = 0;
                report.modifier = 0;
                ble.send(report);
            }

            // mouse
            printf("mouse demo\r\n");
            constexpr size_t NUM_REPS = 250;
            auto start = time_us_64();
            for (size_t i = 0; i < NUM_REPS; i++) {
                hid_mouse_report_t report {};
                report.x = 10;
                ble.send(report);
                report.x = -10;
                ble.send(report);
            }
            auto dt = time_us_64() - start;
            printf("mouse update @ %lldhz\r\n", 1000000 / (dt / NUM_REPS / 2));

            // consumer
            printf("consumer demo\r\n");
            auto press = [&]() {
                hid_consumer_report_t report = HID_USAGE_CONSUMER_PLAY_PAUSE;
                ble.send(report);
                report = 0x0000;
                ble.send(report);
            };
            press();
            vTaskDelay(pdMS_TO_TICKS(1000));
            press();

            // battery
            ble.set_batt(69);
        }
        prev_conn = conn;

        // display passkey if needed
        auto pkey = ble.passkey();
        if (pkey.has_value()) {
            printf("passkey: %06ld\r\n", pkey.value());
        }

        if (ble.connected()) {
            // LED status
            hid_keyboard_report_t report {};
            report.keycode[0] = HID_KEY_CAPS_LOCK;
            ble.send(report);
            report.keycode[0] = 0;
            ble.send(report);
            vTaskDelay(pdMS_TO_TICKS(500));
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, ble.led() & tinyusb::KEYBOARD_LED_CAPSLOCK);
        } else {
            // blinky
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}
