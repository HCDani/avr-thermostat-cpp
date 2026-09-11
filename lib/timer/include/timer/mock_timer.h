#pragma once

#include <stdint.h>

#include "timer/ITimer.h"

namespace timer {

// Test mock. Tests advance time explicitly with fire(), so a run of ticks
// costs no wall-clock time and the schedule is exactly reproducible.
class MockTimer : public ITimer {
public:
    MockTimer();

    void init(ITimerListener* listener) override;
    void start() override;
    void stop() override;

    // Test controls.
    void fire(uint16_t ticks);
    bool running() const;

private:
    ITimerListener* listener_;
    bool running_;
};

}  // namespace timer
