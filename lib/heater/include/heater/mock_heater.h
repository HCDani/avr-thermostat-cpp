#pragma once

#include <stdint.h>

#include "heater/IHeater.h"

namespace heater {

// Test mock. Counts transitions so a test can prove the relay was not
// chattered, which a plain get() cannot show.
class MockHeater : public IHeater {
public:
    MockHeater();

    void init() override;
    void set(bool on) override;
    bool get() const override;

    uint16_t transitions() const;

private:
    bool on_;
    uint16_t transitions_;
};

}  // namespace heater
