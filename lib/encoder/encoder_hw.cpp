#include "encoder_hw.h"

#ifdef __AVR__
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#endif

namespace encoder {

namespace {

// One detent is four Gray-code transitions. Accumulate quarters until ±4.
// Indexed by (previous AB << 2) | current AB. The signs are set so that
// turning the shaft clockwise counts up, which is the opposite of the naive
// table for how A and B land on D6/D7 here.
const int8_t kQuadrature[16] = {0, -1,  1, 0,  1, 0, 0, -1,
                                -1, 0,  0, 1,  0, 1, -1, 0};

}  // namespace

EncoderHw* EncoderHw::active_ = 0;

EncoderHw::EncoderHw() : steps_(0), prev_(0), quarter_(0), stableButton_(false) {}

void EncoderHw::init() {
    active_ = this;
#ifdef __AVR__
    DDRD = static_cast<uint8_t>(DDRD & ~(_BV(PD6) | _BV(PD7)));
    PORTD = static_cast<uint8_t>(PORTD | _BV(PD6) | _BV(PD7));

    DDRB = static_cast<uint8_t>(DDRB & ~_BV(PB4));
    PORTB = static_cast<uint8_t>(PORTB | _BV(PB4));

    prev_ = static_cast<uint8_t>((PIND >> 6) & 0x03);
    steps_ = 0;
    quarter_ = 0;
    stableButton_ = false;

    PCMSK2 = static_cast<uint8_t>(PCMSK2 | _BV(PCINT22) | _BV(PCINT23));
    PCICR = static_cast<uint8_t>(PCICR | _BV(PCIE2));
#endif
}

int16_t EncoderHw::getDelta() {
#ifdef __AVR__
    cli();
    const int16_t delta = steps_;
    steps_ = 0;
    sei();
    return delta;
#else
    return 0;
#endif
}

bool EncoderHw::button() const {
#ifdef __AVR__
    // D12 is PB4, active low.
    const bool raw = (PINB & static_cast<uint8_t>(_BV(PB4))) == 0;
    if (raw != stableButton_) {
        _delay_ms(20);
        const bool again = (PINB & static_cast<uint8_t>(_BV(PB4))) == 0;
        if (again == raw) {
            stableButton_ = raw;
        }
    }
    return stableButton_;
#else
    return false;
#endif
}

void EncoderHw::onPinChange() {
#ifdef __AVR__
    if (active_ == 0) {
        return;
    }
    const uint8_t curr = static_cast<uint8_t>((PIND >> 6) & 0x03);
    const uint8_t index = static_cast<uint8_t>((active_->prev_ << 2) | curr);
    const int8_t dir = kQuadrature[index];
    active_->prev_ = curr;
    if (dir == 0) {
        return;
    }
    active_->quarter_ = static_cast<int8_t>(active_->quarter_ + dir);
    if (active_->quarter_ >= 4) {
        active_->steps_ = static_cast<int16_t>(active_->steps_ + 1);
        active_->quarter_ = 0;
    } else if (active_->quarter_ <= -4) {
        active_->steps_ = static_cast<int16_t>(active_->steps_ - 1);
        active_->quarter_ = 0;
    }
#else
#endif
}

}  // namespace encoder

#ifdef __AVR__
ISR(PCINT2_vect) { encoder::EncoderHw::onPinChange(); }
#endif
