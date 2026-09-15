#pragma once

#include "ITimer.h"

namespace timer {

// The 1 Hz system tick from section 2 of the specification, built on Timer 0
// in CTC mode with a software divider. Timer 1 is hardware PWM for the servo
// on OC1A (D9). Timer 2 is unused.
//
// There is one Timer 0 and one compare-match vector, so the registered
// listener is process-wide and one instance is active at a time.
class TimerHw : public ITimer {
public:
    void init(ITimerListener* listener) override;
    void start() override;
    void stop() override;
};

}  // namespace timer
