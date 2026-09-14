#pragma once

#include <stdint.h>

#include "ITemperature.h"

namespace temp {

// Test mock: an implementation of the same interface the application uses, so
// the application cannot tell it apart from the hardware driver. start()
// latches the staged sample, standing in for the conversion-complete
// interrupt, and get() converts it exactly as the hardware driver does.
class MockTemperature : public ITemperature {
public:
    MockTemperature();

    void init() override;
    void start() override;
    bool available() override;
    uint16_t readRaw() const override;
    Reading celsius(uint16_t raw) const override;
    Reading get() const override;

    // Test controls. setSample stages what the next start() will latch.
    void setSample(uint16_t raw);
    uint16_t starts() const;

private:
    uint16_t sample_;
    uint16_t latestRaw_;
    uint16_t starts_;
    bool ready_;
};

}  // namespace temp
