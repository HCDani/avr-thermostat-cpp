#pragma once

#include <stdint.h>

namespace display {

// Numeric display on LCD row 1. setHeading is the left label (TEMP, TLOW,
// THIGH). showUInt is whole degrees, right-justified before " C". The tick
// marks a refresh; I2C happens in service(). showUInt also marks a refresh
// so encoder edits appear without waiting for the next second.
class IDisplay {
public:
    virtual ~IDisplay() = default;

    virtual void init() = 0;
    virtual void setHeading(const char* heading) = 0;
    virtual void showUInt(uint16_t value) = 0;

    virtual void onTick() = 0;
    virtual void service() = 0;
};

}  // namespace display
