#pragma once

#include <stdint.h>

#include "thermostat/Types.h"

namespace thermostat {
namespace testing {

// Stands in for the board in host tests. Header only, so it costs the target
// nothing: nothing on the AVR side ever includes it.
class FakeHal {
public:
    AdcCount readThermistor() {
        ++reads_;
        return sample_;
    }

    void setHeater(bool on) {
        if (on != heater_) {
            ++transitions_;
        }
        heater_ = on;
    }

    void setSample(AdcCount sample) { sample_ = sample; }
    void setSample(uint16_t sample) { sample_ = AdcCount(sample); }

    bool heater() const { return heater_; }
    uint16_t transitions() const { return transitions_; }
    uint16_t reads() const { return reads_; }

private:
    AdcCount sample_{512};
    bool heater_{false};
    uint16_t transitions_{0};
    uint16_t reads_{0};
};

}  // namespace testing
}  // namespace thermostat
