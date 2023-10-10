#include <iostream>

// from https://miscsolutions.wordpress.com/2019/12/27/using-exceptions-in-c-embedded-software/
[[noreturn]] static void terminate() noexcept {
    while (1);
}

namespace __cxxabiv1 {
    std::terminate_handler __terminate_handler = terminate;
}
