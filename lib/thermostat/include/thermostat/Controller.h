#pragma once

#include <stdint.h>

#include "thermostat/Thermistor.h"
#include "thermostat/Types.h"

namespace thermostat {

struct SetpointLimits {
    DeciCelsius minimum;
    DeciCelsius maximum;
};

struct Config {
    DeciCelsius setpoint;
    DeciCelsius hysteresis;
    SetpointLimits limits;
    // Consecutive out-of-range samples tolerated before the heater is cut.
    // One bad reading is noise; several in a row is a broken sensor.
    uint8_t faultSampleLimit;
};

// Closed-loop thermostat: read the thermistor, decide, drive the relay.
//
// Hal is a template parameter rather than an abstract base class so the call
// is direct and there is no vtable, which matters on a part with 2 KB of SRAM.
// The same logic then compiles against a fake on the host with no #ifdef in
// this file. Any type used as Hal must provide:
//
//     AdcCount readThermistor();
//     void setHeater(bool on);
//
template <class Hal>
class Controller {
public:
    enum class State : uint8_t { Idle, Heating, Fault };

    Controller(Hal& hal, const Config& config) : hal_(hal), config_(config) {
        clampSetpoint();
    }

    // Call at the control rate. Everything here is integer arithmetic on
    // tenths of a degree.
    void tick() {
        const AdcCount sample = hal_.readThermistor();

        if (!Thermistor::inRange(sample)) {
            if (badSamples_ < UINT8_MAX) {
                ++badSamples_;
            }
            if (badSamples_ >= config_.faultSampleLimit) {
                state_ = State::Fault;
                hal_.setHeater(false);
            }
            return;
        }

        badSamples_ = 0;
        temperature_ = Thermistor::convert(sample);

        if (state_ == State::Fault) {
            state_ = State::Idle;
        }

        if (state_ == State::Heating) {
            if (temperature_ >= config_.setpoint + config_.hysteresis) {
                hal_.setHeater(false);
                state_ = State::Idle;
            }
        } else if (temperature_ <= config_.setpoint - config_.hysteresis) {
            hal_.setHeater(true);
            state_ = State::Heating;
        }
    }

    State state() const { return state_; }
    DeciCelsius temperature() const { return temperature_; }
    DeciCelsius setpoint() const { return config_.setpoint; }

    void setSetpoint(DeciCelsius value) {
        config_.setpoint = value;
        clampSetpoint();
    }

private:
    void clampSetpoint() {
        if (config_.setpoint < config_.limits.minimum) {
            config_.setpoint = config_.limits.minimum;
        } else if (config_.setpoint > config_.limits.maximum) {
            config_.setpoint = config_.limits.maximum;
        }
    }

    Hal& hal_;
    Config config_;
    DeciCelsius temperature_{};
    State state_{State::Idle};
    uint8_t badSamples_{0};
};

}  // namespace thermostat
