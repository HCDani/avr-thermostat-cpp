#pragma once

#include <stdint.h>

#include "ITemperature.h"
#include "DeciCelsius.h"
#include "ITimer.h"
#include "IRelay.h"

namespace controller {

struct SetpointLimits {
    temp::DeciCelsius minimum;
    temp::DeciCelsius maximum;
};

struct Config {
    temp::DeciCelsius setpoint;
    temp::DeciCelsius hysteresis;
    SetpointLimits limits;
    // Consecutive out-of-range samples tolerated before the heater is cut.
    // One bad reading is noise; several in a row is a broken sensor.
    uint8_t faultSampleLimit;
};

// Closed-loop thermostat: sample the thermistor, decide, drive the relay.
//
// Application level, so it sees only the driver interfaces and never a pin, a
// register or a concrete driver. That is what lets the host tests run this
// exact logic against the mocks.
//
// The work is split across the two contexts the specification demands:
// onTick() runs in interrupt context and only starts a conversion, while
// service() runs in the main loop and does the arithmetic and the switching.
class Controller : public timer::ITimerListener {
public:
    enum class State : uint8_t { Idle, Heating, Fault };

    Controller(relay::IRelay& relay, temp::ITemperature& sensor, const Config& config);

    // timer::ITimerListener. Interrupt context: start one conversion, return.
    void onTick() override;

    // Main loop. Converts a latched sample when ITemperature::available()
    // reports one, then acts on it. Extra ticks without a completed
    // conversion do nothing; extra completed samples without a service()
    // coalesce into one decision.
    void service();

    State state() const;
    temp::DeciCelsius temperature() const;
    temp::DeciCelsius setpoint() const;

    void setSetpoint(temp::DeciCelsius value);

private:
    void decide(temp::Reading reading);
    void clampSetpoint();

    relay::IRelay& relay_;
    temp::ITemperature& sensor_;
    Config config_;
    temp::DeciCelsius temperature_;
    State state_;
    uint8_t badSamples_;
};

}  // namespace controller
