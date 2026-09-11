#include "temp/Conversion.h"

#include "thermostat/Thermistor.h"
#include "thermostat/Types.h"

namespace temp {

Reading convert(uint16_t raw) {
    const thermostat::AdcCount count(raw);

    Reading reading;
    if (thermostat::Thermistor::inRange(count)) {
        reading.status = SensorStatus::Ok;
        reading.value = thermostat::Thermistor::convert(count);
        return reading;
    }

    reading.status =
        raw < thermostat::Thermistor::minValidCount() ? SensorStatus::Open : SensorStatus::Shorted;
    reading.value = thermostat::DeciCelsius();
    return reading;
}

}  // namespace temp
