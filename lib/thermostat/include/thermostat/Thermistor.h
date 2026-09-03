#pragma once

#include <stdint.h>

#include "thermostat/ThermistorTable.h"
#include "thermostat/Types.h"

namespace thermostat {

// Converts a raw ADC count into a temperature by interpolating the generated
// table. Pure and target-independent, so every case below is exercised by the
// host suite rather than by holding a lighter near the sensor.
class Thermistor {
public:
    static constexpr uint16_t minValidCount() { return table::kMinValidCount; }
    static constexpr uint16_t maxValidCount() { return table::kMaxValidCount; }

    // False means the divider is telling us about a disconnected or shorted
    // thermistor, not about the room.
    static constexpr bool inRange(AdcCount count) {
        return count.raw() >= table::kMinValidCount && count.raw() <= table::kMaxValidCount;
    }

    // Undefined for counts outside the valid window; check inRange() first.
    static DeciCelsius convert(AdcCount count) {
        const uint16_t raw = count.raw();
        const uint16_t index = static_cast<uint16_t>(raw / table::kStep);
        const uint16_t offset = static_cast<uint16_t>(raw % table::kStep);

        const int16_t lower = table::entry(index);
        if (offset == 0) {
            return DeciCelsius(lower);
        }

        const int16_t upper = table::entry(static_cast<uint16_t>(index + 1));
        const int32_t span = static_cast<int32_t>(upper) - static_cast<int32_t>(lower);
        const int32_t interpolated =
            static_cast<int32_t>(lower) + (span * static_cast<int32_t>(offset)) / table::kStep;
        return DeciCelsius(static_cast<int16_t>(interpolated));
    }
};

}  // namespace thermostat
