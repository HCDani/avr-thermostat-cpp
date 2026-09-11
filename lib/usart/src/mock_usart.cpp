#include "usart/mock_usart.h"

namespace usart {

MockUsart::MockUsart() : initialised_(false), writes_(0), last_(0) {}

void MockUsart::init() { initialised_ = true; }

void MockUsart::write(uint8_t byte) {
    last_ = byte;
    if (writes_ < UINT16_MAX) {
        ++writes_;
    }
}

bool MockUsart::initialised() const { return initialised_; }

uint16_t MockUsart::writes() const { return writes_; }

uint8_t MockUsart::last() const { return last_; }

}  // namespace usart
