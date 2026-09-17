#include "Controller.h"

namespace controller {

Controller::Controller(relay::IRelay& relay, temp::ITemperature& sensor, const Config& config)
    : relay_(relay),
      sensor_(sensor),
      config_(config),
      temperature_(),
      state_(State::Idle),
      badSamples_(0) {
    clampSetpoint();
    sensor_.init();
}

void Controller::onTick() { sensor_.start(); }

void Controller::service() {
    if (!sensor_.available()) {
        return;
    }

    // The conversion happens here, in loop context, not in either ISR.
    decide(sensor_.get());
}

void Controller::decide(temp::Reading reading) {
    if (reading.status != temp::SensorStatus::Ok) {
        if (badSamples_ < UINT8_MAX) {
            ++badSamples_;
        }
        if (badSamples_ >= config_.faultSampleLimit) {
            state_ = State::Fault;
            relay_.set(false);
        }
        return;
    }

    badSamples_ = 0;
    temperature_ = reading.value;

    if (state_ == State::Fault) {
        state_ = State::Idle;
    }

    if (state_ == State::Heating) {
        if (temperature_ >= config_.setpoint + config_.hysteresis) {
            relay_.set(false);
            state_ = State::Idle;
        }
    } else if (temperature_ <= config_.setpoint - config_.hysteresis) {
        relay_.set(true);
        state_ = State::Heating;
    }
}

Controller::State Controller::state() const { return state_; }

temp::DeciCelsius Controller::temperature() const { return temperature_; }

temp::DeciCelsius Controller::setpoint() const { return config_.setpoint; }

void Controller::setSetpoint(temp::DeciCelsius value) {
    config_.setpoint = value;
    clampSetpoint();
}

void Controller::clampSetpoint() {
    if (config_.setpoint < config_.limits.minimum) {
        config_.setpoint = config_.limits.minimum;
    } else if (config_.setpoint > config_.limits.maximum) {
        config_.setpoint = config_.limits.maximum;
    }
}

}  // namespace controller
