#pragma once

#include <stdint.h>

namespace display {

// Numeric display on LCD row 1. showUInt takes tenths of a degree. The left
// ten characters are the bar range "BAR 21-28C"; the rest is e.g. " 21.5C".
// The tick only marks a refresh; I2C happens in service().
class IDisplay {
public:
    virtual ~IDisplay() = default;

    virtual void init() = 0;
    virtual void showUInt(uint16_t value) = 0;

    // Interrupt context: set the pending-refresh flag, return.
    virtual void onTick() = 0;

    // Main loop. Writes row 1 if a tick has marked a refresh.
    virtual void service() = 0;
};

}  // namespace display
