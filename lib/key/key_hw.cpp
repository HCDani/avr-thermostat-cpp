#include "key_hw.h"

#ifdef __AVR__
#include <avr/io.h>
#include <util/delay.h>
#endif

namespace key {

KeyHw::KeyHw() {
    for (uint8_t i = 0; i < 3; ++i) {
        stable_[i] = false;
    }
}

void KeyHw::init() {
#ifdef __AVR__
    DDRD = static_cast<uint8_t>(DDRD & ~(_BV(PD2) | _BV(PD3) | _BV(PD4)));
    PORTD = static_cast<uint8_t>(PORTD & ~(_BV(PD2) | _BV(PD3) | _BV(PD4)));
#endif
}

uint8_t KeyHw::pinBit(uint8_t keyNo) const {
    const uint8_t bits[3] = {
#ifdef __AVR__
        PD2, PD3, PD4
#else
        2, 3, 4
#endif
    };
    return bits[keyNo - 1];
}

bool KeyHw::read(uint8_t keyNo) const {
#ifdef __AVR__
    return (PIND & static_cast<uint8_t>(_BV(pinBit(keyNo)))) != 0;
#else
    (void)keyNo;
    return false;
#endif
}

bool KeyHw::get(uint8_t keyNo) const {
#ifdef __AVR__
    const bool raw = read(keyNo);
    bool& stable = stable_[keyNo - 1];
    if (raw != stable) {
        _delay_ms(20);
        if (read(keyNo) == raw) {
            stable = raw;
        }
    }
    return stable;
#else
    (void)keyNo;
    return false;
#endif
}

}  // namespace key
