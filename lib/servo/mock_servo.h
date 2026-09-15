#pragma once

#include <stdint.h>

#include "IServo.h"

namespace servo {

class MockServo : public IServo {
public:
    MockServo();

    void init() override;
    void setAngle(uint16_t angle) override;

    bool inited() const;
    uint16_t angle() const;

private:
    uint16_t angle_;
    bool inited_;
};

}  // namespace servo
