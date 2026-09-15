#include "Thermometer.h"

namespace thermo {

namespace {

// Spec range in tenths of a degree: position 1 at 21.0 C, all eight at 28.0 C.
const int16_t kBarLo = 210;
const int16_t kBarHi = 280;

}  // namespace

Thermometer::Thermometer(temp::ITemperature& sensor, led::ILed& leds, display::IDisplay& display)
    : sensor_(sensor), leds_(leds), display_(display), temperature_() {}

void Thermometer::init() {
    leds_.init();
    display_.init();
    sensor_.init();
}

void Thermometer::onTick() {
    sensor_.start();
    display_.onTick();
}

void Thermometer::service() {
    if (sensor_.available()) {
        const temp::Reading reading = sensor_.get();
        if (reading.status == temp::SensorStatus::Ok) {
            temperature_ = reading.value;
            leds_.bar(kBarLo, kBarHi, temperature_.raw());
            if (temperature_.raw() >= 0) {
                display_.showUInt(static_cast<uint16_t>(temperature_.raw()));
            }
        }
    }

    display_.service();
}

temp::DeciCelsius Thermometer::temperature() const { return temperature_; }

}  // namespace thermo
