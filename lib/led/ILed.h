#pragma once

#include <stdint.h>

namespace led {

// Status indicators on LCD row 2. ledNo 1..8 maps to columns 0..7.
class ILed {
public:
    virtual ~ILed() = default;

    virtual void init() = 0;
    virtual void set(uint8_t ledNo, bool state) = 0;
    virtual bool get(uint8_t ledNo) const = 0;
};

}  // namespace led
