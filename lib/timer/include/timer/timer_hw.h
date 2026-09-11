#pragma once

#include "timer/ITimer.h"

namespace timer {

// The 1 Hz system tick from section 2 of the specification, built on Timer 0
// in CTC mode with a software divider. Timer 1 is reserved for servo pulse
// timing and Timer 2 for the servo pin's compare output, so the tick gets the
// timer nothing else wants.
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
