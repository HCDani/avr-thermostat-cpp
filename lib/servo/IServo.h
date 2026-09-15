#pragma once

#include <stdint.h>

namespace servo {

class IServo {
public:
    virtual ~IServo() = default;

    virtual void init() = 0;
    virtual void setAngle(uint16_t angle) = 0;
};

}  // namespace servo
