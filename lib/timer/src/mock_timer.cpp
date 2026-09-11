#include "timer/mock_timer.h"

namespace timer {

MockTimer::MockTimer() : listener_(0), running_(false) {}

void MockTimer::init(ITimerListener* listener) { listener_ = listener; }

void MockTimer::start() { running_ = true; }

void MockTimer::stop() { running_ = false; }

// A stopped timer delivers nothing, which is what makes stop() testable.
void MockTimer::fire(uint16_t ticks) {
    for (uint16_t i = 0; i < ticks; ++i) {
        if (!running_ || listener_ == 0) {
            return;
        }
        listener_->onTick();
    }
}

bool MockTimer::running() const { return running_; }

}  // namespace timer
