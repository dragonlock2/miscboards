#pragma once

#include <cstdint>
#include <tuple>
#include "eth.h"

class USB {
public:
    USB(eth::OASPI &oaspi, eth::Eth &eth); // singleton!
    ~USB();

    USB(USB&) = delete;
    USB(USB&&) = delete;

    std::tuple<uint32_t, uint32_t> get_error(); // tx drop, rx drop

    eth::OASPI &_oaspi;
    eth::Eth &_eth;
};
