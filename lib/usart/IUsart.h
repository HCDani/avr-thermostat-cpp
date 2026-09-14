#pragma once

#include <stdint.h>

namespace usart {

// Polled USART 0. Unity's host runner reads this as the test transport; the
// application may use the same interface later. Transmit is blocking: the
// caller waits until the data register is empty.
class IUsart {
public:
    virtual ~IUsart() = default;

    virtual void init() = 0;
    virtual void write(uint8_t byte) = 0;
};

}  // namespace usart
