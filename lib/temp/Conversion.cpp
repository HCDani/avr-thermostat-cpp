#include "Conversion.h"

#include "ThermistorTable.h"
#include "DeciCelsius.h"

namespace temp {

Reading convert(uint16_t adcvalue) {
    Reading reading;
    if (adcvalue >= table::kMinValidCount && adcvalue <= table::kMaxValidCount) {
        reading.status = SensorStatus::Ok;

        const uint16_t index = static_cast<uint16_t>(adcvalue / table::kStep);
        const uint16_t offset = static_cast<uint16_t>(adcvalue % table::kStep);
    
        const int16_t lower = table::entry(index);
        if (offset == 0) {
            reading.value = DeciCelsius(lower);
            return reading;
        }
    
        const int16_t upper = table::entry(static_cast<uint16_t>(index + 1));
        const int32_t span = static_cast<int32_t>(upper) - static_cast<int32_t>(lower);
        const int32_t interpolated = static_cast<int32_t>(lower) + (span * static_cast<int32_t>(offset)) / table::kStep;
        
        reading.value =  DeciCelsius(static_cast<int16_t>(interpolated));

        return reading;
    }

    reading.status = adcvalue < table::kMinValidCount ? SensorStatus::Open : SensorStatus::Shorted;
    reading.value = DeciCelsius();
    return reading;
}

}  // namespace temp
