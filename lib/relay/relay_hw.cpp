#include "relay_hw.h"

#ifdef __AVR__
#include <avr/io.h>
#endif

namespace relay {

RelayHw::RelayHw() : on_(false) {}

void RelayHw::init() {
#ifdef __AVR__
    // D5 = PD5, Grove relay SIG. D13 = PB5, the Uno LED. Both outputs.
    DDRD = static_cast<uint8_t>(DDRD | _BV(PD5));
    DDRB = static_cast<uint8_t>(DDRB | _BV(PB5));
#endif
    set(false);
}

void RelayHw::set(bool on) {
    on_ = on;
#ifdef __AVR__
    if (on) {
        PORTD = static_cast<uint8_t>(PORTD | _BV(PD5));
        PORTB = static_cast<uint8_t>(PORTB | _BV(PB5));
    } else {
        PORTD = static_cast<uint8_t>(PORTD & ~_BV(PD5));
        PORTB = static_cast<uint8_t>(PORTB & ~_BV(PB5));
    }
#endif
}

bool RelayHw::get() const { return on_; }

}  // namespace relay
