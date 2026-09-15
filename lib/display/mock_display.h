#pragma once

#include <stdint.h>

#include "IDisplay.h"

namespace display {

class MockDisplay : public IDisplay {
public:
    MockDisplay();

    void init() override;
    void showUInt(uint16_t value) override;
    void onTick() override;
    void service() override;

    bool inited() const;
    uint16_t latched() const;
    uint16_t shown() const;
    bool flushed() const;

private:
    uint16_t value_;
    uint16_t shown_;
    bool pending_;
    bool inited_;
    bool flushed_;
};

}  // namespace display
