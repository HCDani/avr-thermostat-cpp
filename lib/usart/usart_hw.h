#pragma once

#include "IUsart.h"

namespace usart {

// USART0 in asynchronous 8N1 at 115200 baud, the USB-serial port on the Uno.
// Transmit is polled; there is no RX and no interrupt. One USART0 on the
// part, so one instance is meaningful.
class UsartHw : public IUsart {
public:
    void init() override;
    void write(uint8_t byte) override;
};

}  // namespace usart
