#include "mock_servo.h"

namespace servo {

MockServo::MockServo() : angle_(0), inited_(false) {}

void MockServo::init() { inited_ = true; }

void MockServo::setAngle(uint16_t angle) { angle_ = angle; }

bool MockServo::inited() const { return inited_; }

uint16_t MockServo::angle() const { return angle_; }

}  // namespace servo
