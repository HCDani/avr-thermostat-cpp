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

    // Lights positions 1..8 as a filled bar. Position 1 on if value >= lo,
    // all eight on if value >= hi, and the rest in proportion.
    virtual void bar(int16_t lo, int16_t hi, int16_t value) = 0;

    // Writes s on row 2 starting at col (0..15). The bar owns 0..7.
    virtual void write(uint8_t col, const char* s) = 0;
};

}  // namespace led
