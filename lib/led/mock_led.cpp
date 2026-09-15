#include "mock_led.h"

namespace led {

MockLed::MockLed() : inited_(false) {
    for (uint8_t i = 0; i < 8; ++i) {
        state_[i] = false;
    }
}

void MockLed::init() { inited_ = true; }

void MockLed::set(uint8_t ledNo, bool state) { state_[ledNo - 1] = state; }

bool MockLed::get(uint8_t ledNo) const { return state_[ledNo - 1]; }

bool MockLed::inited() const { return inited_; }

}  // namespace led
