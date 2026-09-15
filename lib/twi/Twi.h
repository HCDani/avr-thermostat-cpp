#pragma once

#include <stdint.h>

namespace twi {

// Blocking I2C master. The header stays AVR-free so LcdHw can name the bus
// without pulling avr-libc into the interface layer.
//
// Every wait for the hardware is bounded. An absent or unresponsive device
// must not spin the main loop forever, so a transfer that stalls is abandoned
// and recorded in ok()/status() rather than hanging the firmware.
class Twi {
public:
    Twi();

    void init();
    void write(uint8_t address, const uint8_t* data, uint8_t length);

    // State of the most recent write(): true when every byte was acknowledged.
    bool ok() const;

    // TWSR status bits of the most recent write, for diagnosing a failure.
    uint8_t status() const;

private:
    bool ok_;
    uint8_t status_;
};

}  // namespace twi
