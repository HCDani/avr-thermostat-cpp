#pragma once

#include <stdint.h>

#include "thermostat/ThermistorTable.h"
#include "thermostat/Types.h"

namespace thermostat {

// Converts a raw ADC count into a temperature by interpolating the generated table.
// Pure and target-independent, so every case below is exercised by the host suite rather than by holding a lighter near the sensor.
class Thermistor {
public:
    static constexpr uint16_t minValidCount() { return table::kMinValidCount; }
    static constexpr uint16_t maxValidCount() { return table::kMaxValidCount; }

    // False means the divider is telling us about a disconnected or shorted thermistor, not about the room.
    static constexpr bool inRange(AdcCount count) {
        return count.raw() >= table::kMinValidCount && count.raw() <= table::kMaxValidCount;
    }

    // Undefined for counts outside the valid window; check inRange() first.
    static DeciCelsius convert(AdcCount count);
};

}  // namespace thermostat
