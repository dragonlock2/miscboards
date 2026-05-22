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

    eth::OASPI &oaspi_;
    eth::Eth &eth_;
};
