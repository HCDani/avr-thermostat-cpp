#pragma once

#include <stdint.h>

#include "Adc.h"
#include "ITemperature.h"

namespace temp {

// NTC on an analog input, sampled through the ADC conversion-complete
// interrupt. The part has one ADC and one such vector, so one instance is
// active at a time; init() and start() register this one.
class TemperatureHw : public ITemperature {
public:
    TemperatureHw(adc::Adc& adc, uint8_t pin);

    void init() override;
    void start() override;
    bool available() override;
    uint16_t readRaw() const override;
    Reading celsius(uint16_t raw) const override;
    Reading get() const override;

private:
    // Handed to the ADC as a plain function pointer, so it cannot be a member
    // function and has to find its way back through active_.
    static void onConversionComplete(uint16_t adcvalue);

    adc::Adc& adc_;
    uint8_t pin_;
    volatile uint16_t latestRaw_;
    volatile bool ready_;

    static TemperatureHw* active_;
};

}  // namespace temp
