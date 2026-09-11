// Target entry point. The 1 Hz tick starts an ADC conversion, the
// conversion-complete interrupt latches the count, and the main loop turns it
// into a temperature when ITemperature::available() reports a new sample.
// That split is the whole sampling chain from section 2 of the specification:
// no arithmetic in either ISR.
//
// A Controller cannot be built here yet: only the mock implements IHeater, so
// there is nothing to switch the relay with. Until then this converts the
// latest reading so the drivers are exercised end to end on the part.
#if defined(__AVR__) && !defined(PIO_UNIT_TESTING)

#include <avr/interrupt.h>
#include <stdint.h>

#include "hal/Adc.h"
#include "temp/temp_hw.h"
#include "timer/timer_hw.h"

namespace {

// The thermistor divider sits on A0, port 1 of the Grove base shield.
const hal::PinId kThermistorPin = 0;

volatile int16_t g_latestDeciCelsius = 0;
volatile bool g_sensorFaulted = false;

class Sampler : public timer::ITimerListener {
public:
    explicit Sampler(temp::ITemperature& sensor) : sensor_(sensor) {}

    void onTick() override { sensor_.start(); }

private:
    temp::ITemperature& sensor_;
};

}  // namespace

int main() {
    hal::Adc adc;
    temp::TemperatureHw sensor(adc, kThermistorPin);
    timer::TimerHw clock;

    Sampler sampler(sensor);

    sensor.init();
    clock.init(&sampler);

    sei();
    clock.start();

    for (;;) {
        if (sensor.available()) {
            // Loop context, so the table interpolation is allowed here.
            const temp::Reading reading = sensor.get();
            g_sensorFaulted = reading.status != temp::SensorStatus::Ok;
            if (!g_sensorFaulted) {
                g_latestDeciCelsius = reading.value.raw();
            }
        }
    }
}

#endif
