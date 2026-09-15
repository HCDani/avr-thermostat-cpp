#pragma once

#include "IDisplay.h"
#include "ILed.h"
#include "ITemperature.h"
#include "ITimer.h"
#include "DeciCelsius.h"

namespace thermo {

// Part 2 + 3: 1 Hz sample of the NTC. Bar on LCD row 2 spanning 21–28 C;
// row 1 shows BAR 21-28C and the reading as e.g. " 21.5C".
class Thermometer : public timer::ITimerListener {
public:
    Thermometer(temp::ITemperature& sensor, led::ILed& leds, display::IDisplay& display);

    void init();

    // Interrupt context: start one conversion and mark a display refresh.
    void onTick() override;

    // Main loop. Converts a latched sample when available() is true, fills
    // the bar, latches the numeric value, then lets the display write row 1.
    void service();

    temp::DeciCelsius temperature() const;

private:
    temp::ITemperature& sensor_;
    led::ILed& leds_;
    display::IDisplay& display_;
    temp::DeciCelsius temperature_;
};

}  // namespace thermo
