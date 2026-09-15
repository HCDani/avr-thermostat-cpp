#include "Twi.h"

namespace twi {

Twi::Twi() : ok_(false), status_(0) {}

bool Twi::ok() const { return ok_; }

uint8_t Twi::status() const { return status_; }

}  // namespace twi

#ifdef __AVR__

#include <avr/io.h>

namespace {

// TWSR values the transfer must see, master transmitter.
const uint8_t kStartSent = 0x08;
const uint8_t kAddressAcked = 0x18;
const uint8_t kDataAcked = 0x28;

// Bound on one byte. A byte at 100 kHz needs about 90 us; this allows far
// longer, so it only expires when the hardware is not going to finish at all
// (bus held low, no pull-ups, nothing driving SDA).
const uint16_t kGuard = 0xFFFF;

bool waitForFlag() {
    uint16_t guard = kGuard;
    while ((TWCR & static_cast<uint8_t>(_BV(TWINT))) == 0) {
        if (--guard == 0) {
            return false;
        }
    }
    return true;
}

uint8_t busStatus() { return static_cast<uint8_t>(TWSR & 0xF8); }

void stop() { TWCR = static_cast<uint8_t>(_BV(TWINT) | _BV(TWSTO) | _BV(TWEN)); }

}  // namespace

namespace twi {

void Twi::init() {
    // SDA and SCL stay inputs and get their internal pull-ups. A Grove module
    // brings its own, but without any the lines never rise and the hardware
    // cannot finish a START at all.
    DDRC = static_cast<uint8_t>(DDRC & ~(_BV(PC4) | _BV(PC5)));
    PORTC = static_cast<uint8_t>(PORTC | _BV(PC4) | _BV(PC5));

    // 100 kHz on a 16 MHz part, prescaler 1: TWBR = ((F_CPU / 100000) - 16) / 2.
    TWSR = 0;
    TWBR = 72;
    TWCR = static_cast<uint8_t>(_BV(TWEN));
}

void Twi::write(uint8_t address, const uint8_t* data, uint8_t length) {
    ok_ = false;

    TWCR = static_cast<uint8_t>(_BV(TWINT) | _BV(TWSTA) | _BV(TWEN));
    if (!waitForFlag()) {
        status_ = 0xFF;  // Timed out before the flag ever set.
        stop();
        return;
    }
    status_ = busStatus();
    if (status_ != kStartSent) {
        stop();
        return;
    }

    TWDR = static_cast<uint8_t>(address << 1);
    TWCR = static_cast<uint8_t>(_BV(TWINT) | _BV(TWEN));
    if (!waitForFlag()) {
        status_ = 0xFF;
        stop();
        return;
    }
    status_ = busStatus();
    if (status_ != kAddressAcked) {
        stop();
        return;
    }

    for (uint8_t i = 0; i < length; ++i) {
        TWDR = data[i];
        TWCR = static_cast<uint8_t>(_BV(TWINT) | _BV(TWEN));
        if (!waitForFlag()) {
            status_ = 0xFF;
            stop();
            return;
        }
        status_ = busStatus();
        if (status_ != kDataAcked) {
            stop();
            return;
        }
    }

    stop();
    ok_ = true;
}

}  // namespace twi

#else

namespace twi {

void Twi::init() {}

void Twi::write(uint8_t, const uint8_t*, uint8_t) {}

}  // namespace twi

#endif
