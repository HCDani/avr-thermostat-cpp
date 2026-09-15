#pragma once

#include <stdint.h>

#include "IServo.h"

namespace servo {

// Timer 1 hardware PWM on OC1A (D9). Pulse 1 ms at 0 deg, 2 ms at 180 deg,
// 20 ms period.
class ServoHw : public IServo {
public:
    ServoHw();

    void init() override;
    void setAngle(uint16_t angle) override;

private:
    uint16_t angle_;
};

}  // namespace servo
