#pragma once

#include <stdint.h>

namespace adc {

// Runs in the conversion-complete interrupt, so it must be short and must not
// block. A plain function pointer rather than std::function because there is no
// heap on the target. The argument is the 10-bit result; this header does not
// name any thermostat type.
using ConversionCallback = void (*)(uint16_t adcvalue);

// Interrupt-driven single conversion. start() selects the pin, kicks off the
// hardware and returns immediately; the callback fires once the result is
// latched, so the control loop never spins waiting on the ADC.
//
// The part has one ADC and one conversion-complete vector. The object is a
// handle; the registered callback is process-wide.
class Adc {
public:
    void start(uint8_t pin, ConversionCallback callback);
};

}  // namespace adc
