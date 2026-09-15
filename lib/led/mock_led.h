#pragma once

#include <stdint.h>

#include "ILed.h"

namespace led {

class MockLed : public ILed {
public:
    MockLed();

    void init() override;
    void set(uint8_t ledNo, bool state) override;
    bool get(uint8_t ledNo) const override;
    void bar(int16_t lo, int16_t hi, int16_t value) override;
    void write(uint8_t col, const char* s) override;

    bool inited() const;
    const char* caption() const;

private:
    bool state_[8];
    char caption_[9];
    bool inited_;
};

}  // namespace led
