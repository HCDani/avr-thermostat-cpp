#include "thermostat/Thermistor.h"

namespace thermostat {

DeciCelsius Thermistor::convert(AdcCount count) {
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

}  // namespace thermostat
