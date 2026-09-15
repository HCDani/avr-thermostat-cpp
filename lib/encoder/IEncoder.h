#pragma once

#include <stdint.h>

namespace encoder {

// Incremental encoder on D6/D7. The push button is D12, active low with the
// AVR pull-up. getDelta is steps since the last call; clockwise is positive.
class IEncoder {
public:
    virtual ~IEncoder() = default;

    virtual void init() = 0;
    virtual int16_t getDelta() = 0;
    virtual bool button() const = 0;
};

}  // namespace encoder
