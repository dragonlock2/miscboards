#include <string>
#include <FreeRTOS.h>
#include <task.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>
#include "ble.h"
#include "usb.h"

#include <class/hid/hid.h> // TinyUSB keycodes

static void demo(HID &dev, const char *name) {
    // keyboard
    printf("%s keyboard demo\r\n", name);
    static const uint8_t ASCII_TO_CODE[128][2] = { HID_ASCII_TO_KEYCODE };
    std::string s = "Hello World!";
    for (char &c: s) {
        HID::keyboard_report_t report {};
        report.keycode[0] = ASCII_TO_CODE[static_cast<uint8_t>(c)][1];
        report.modifier = ASCII_TO_CODE[static_cast<uint8_t>(c)][0] ? KEYBOARD_MODIFIER_LEFTSHIFT : 0;
        dev.send(report);
        report.keycode[0] = 0;
        report.modifier = 0;
        dev.send(report);
    }

    // mouse
    printf("%s mouse demo\r\n", name);
    constexpr size_t NUM_REPS = 250;
    auto start = time_us_64();
    for (size_t i = 0; i < NUM_REPS; i++) {
        HID::mouse_report_t report {};
        report.x = 10;
        dev.send(report);
        report.x = -10;
        dev.send(report);
    }
    auto dt = time_us_64() - start;
    printf("%s mouse update @ %lldhz\r\n", name, 1000000 / (dt / NUM_REPS / 2));

    // consumer
    printf("%s consumer demo\r\n", name);
    auto press = [&]() {
        HID::consumer_report_t report = HID_USAGE_CONSUMER_PLAY_PAUSE;
        dev.send(report);
        report = 0x0000;
        dev.send(report);
    };
    press();
    vTaskDelay(pdMS_TO_TICKS(1000));
    press();
}

static void toggle(HID &dev) {
    HID::keyboard_report_t report {};
    report.keycode[0] = HID_KEY_CAPS_LOCK;
    dev.send(report);
    report.keycode[0] = 0;
    dev.send(report);
    vTaskDelay(pdMS_TO_TICKS(500));
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, dev.led() & KEYBOARD_LED_CAPSLOCK);
}

static void usb_demo(void*) {
    USB usb;
    printf("usb initialized!\r\n");

    bool prev_conn = false;
    while (true) {
        // connection demo
        bool conn = usb.connected();
        if (conn && !prev_conn) {
            vTaskDelay(pdMS_TO_TICKS(100));
            demo(usb, "usb");
        }
        prev_conn = conn;

        // toggle demo
        if (usb.connected()) {
            toggle(usb);
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    vTaskDelete(nullptr);
}

void app_main(void*) {
    configASSERT(xTaskCreate(usb_demo, "usb_demo", configMINIMAL_STACK_SIZE,
        NULL, configAPP_MAIN_PRIORITY, NULL) == pdPASS);
    BLE ble(BLE::conn_type::PASSKEY_CONFIRM);
    printf("ble initialized!\r\n");

    bool prev_conn = false;
    while (true) {
        // connection demo
        bool conn = ble.connected();
        if (conn && !prev_conn) {
            // wait settle, may be too short right after pairing
            // if doesn't work still, may need to re-pair
            vTaskDelay(pdMS_TO_TICKS(1000));
            demo(ble, "ble");
            ble.set_batt(69);
        }
        prev_conn = conn;

        // display passkey if needed
        auto pkey = ble.passkey();
        if (pkey.has_value()) {
            printf("passkey: %06ld\r\n", pkey.value());
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        // toggle demo
        if (ble.connected()) {
            toggle(ble);
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    vTaskDelete(nullptr);
}
