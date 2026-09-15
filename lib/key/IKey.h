#pragma once

#include <stdint.h>

namespace key {

// Grove buttons, keys 1..3. Pressed is true. Debounce lives in the hardware
// driver; the Grove modules already have a pull-down, so init does not enable
// the AVR pull-ups.
class IKey {
public:
    virtual ~IKey() = default;

    virtual void init() = 0;
    virtual bool get(uint8_t keyNo) const = 0;
};

}  // namespace key
