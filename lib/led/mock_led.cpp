#include "mock_led.h"

#include <stdint.h>

namespace led {

MockLed::MockLed() : inited_(false) {
    for (uint8_t i = 0; i < 8; ++i) {
        state_[i] = false;
    }
    for (uint8_t i = 0; i < 8; ++i) {
        caption_[i] = ' ';
    }
    caption_[8] = '\0';
}

void MockLed::init() { inited_ = true; }

void MockLed::set(uint8_t ledNo, bool state) { state_[ledNo - 1] = state; }

bool MockLed::get(uint8_t ledNo) const { return state_[ledNo - 1]; }

void MockLed::bar(int16_t lo, int16_t hi, int16_t value) {
    uint8_t lit = 0;
    if (value >= hi) {
        lit = 8;
    } else if (value >= lo) {
        const int32_t span = static_cast<int32_t>(hi) - static_cast<int32_t>(lo);
        for (uint8_t i = 0; i < 8; ++i) {
            const int16_t threshold =
                static_cast<int16_t>(static_cast<int32_t>(lo) + (static_cast<int32_t>(i) * span) / 7);
            if (value >= threshold) {
                lit = static_cast<uint8_t>(i + 1);
            }
        }
    }

    for (uint8_t i = 0; i < 8; ++i) {
        state_[i] = i < lit;
    }
}

void MockLed::write(uint8_t col, const char* s) {
    if (col < 8) {
        return;
    }
    uint8_t i = static_cast<uint8_t>(col - 8);
    while (*s != '\0' && i < 8) {
        caption_[i] = *s;
        ++i;
        ++s;
    }
}

bool MockLed::inited() const { return inited_; }

const char* MockLed::caption() const { return caption_; }

}  // namespace led
