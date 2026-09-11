#pragma once

#include <stdint.h>

#include "usart/IUsart.h"

namespace usart {

class MockUsart : public IUsart {
public:
    MockUsart();

    void init() override;
    void write(uint8_t byte) override;

    bool initialised() const;
    uint16_t writes() const;
    uint8_t last() const;

private:
    bool initialised_;
    uint16_t writes_;
    uint8_t last_;
};

}  // namespace usart
