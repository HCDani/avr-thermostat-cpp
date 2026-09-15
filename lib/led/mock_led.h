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

    bool inited() const;

private:
    bool state_[8];
    bool inited_;
};

}  // namespace led
