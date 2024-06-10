#include <cstdio>
#include <iostream>

namespace __cxxabiv1 {
    std::terminate_handler __terminate_handler = std::abort;
}

extern "C" int main(void) {
    printf("booted!\r\n");

    // TODO freertos
    // TODO led, btn
    // TODO mdio
    // TODO extra conn
    // TODO usb
    // TODO ethernet
}
