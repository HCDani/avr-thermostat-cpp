#include "servo_hw.h"

#ifdef __AVR__
#include <avr/io.h>
#endif

namespace servo {

namespace {

const uint16_t kTop = 39999;
const uint16_t kPulse0 = 2000;
const uint16_t kPulse180 = 4000;

}  // namespace

ServoHw::ServoHw() : angle_(0) {}

void ServoHw::init() {
#ifdef __AVR__
    DDRB = static_cast<uint8_t>(DDRB | _BV(PB1));

    ICR1 = kTop;
    OCR1A = kPulse0;
    TCCR1A = static_cast<uint8_t>(_BV(COM1A1) | _BV(WGM11));
    TCCR1B = static_cast<uint8_t>(_BV(WGM13) | _BV(WGM12) | _BV(CS11));
#endif
    angle_ = 0;
}

void ServoHw::setAngle(uint16_t angle) {
    if (angle > 180) {
        angle = 180;
    }
    angle_ = angle;
#ifdef __AVR__
    const uint32_t span = static_cast<uint32_t>(kPulse180) - kPulse0;
    OCR1A = static_cast<uint16_t>(kPulse0 + (span * angle) / 180U);
#endif
}

}  // namespace servo
