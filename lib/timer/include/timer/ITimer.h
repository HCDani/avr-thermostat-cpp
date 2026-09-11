#pragma once

namespace timer {

// Runs in the compare-match interrupt, so it must be short: start hardware or
// set a flag, never arithmetic or I2C.
class ITimerListener {
public:
    virtual ~ITimerListener() = default;

    virtual void onTick() = 0;
};

// System tick driver. The rate is fixed at 1 Hz by the specification, so it is
// not a parameter: there is one tick in this system and everything periodic
// hangs off it.
class ITimer {
public:
    virtual ~ITimer() = default;

    // Configures the hardware and registers the listener. Does not yet
    // deliver ticks.
    virtual void init(ITimerListener* listener) = 0;

    // Enables and disables delivery.
    virtual void start() = 0;
    virtual void stop() = 0;
};

}  // namespace timer
