#include "timer/timer_hw.h"

// Only the implementation touches registers; timer/ITimer.h stays AVR-free.
#ifdef __AVR__

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

#ifndef F_CPU
#error "F_CPU is not defined; PlatformIO sets it from the board manifest."
#endif

namespace {

// Timer 0 rather than Timer 1. Timer 1 is the only 16-bit counter on the part
// and the servo needs that width for pulse timing. Timer 2 is left alone too,
// because its OC2A output is D11, the servo signal pin.
//
// The cost of an 8-bit counter is that 1 Hz is out of hardware reach: even the
// coarsest prescaler leaves 15625 counts in a second. So the compare unit runs
// at 125 counts, giving 125 interrupts a second, and a software divider turns
// those into the tick. 15625 is 5^6, so 125 divides it exactly and the tick
// does not drift.
const unsigned long kPrescaler = 1024UL;
const unsigned long kTicksPerSecond = F_CPU / kPrescaler;
const unsigned long kCompareValue = 124UL;
const unsigned long kInterruptsPerSecond = kTicksPerSecond / (kCompareValue + 1UL);

static_assert(kCompareValue <= 255UL, "compare value exceeds Timer 0's 8 bits");
static_assert(kTicksPerSecond % (kCompareValue + 1UL) == 0UL,
              "compare value does not divide the clock exactly, so the tick would drift");
static_assert(kInterruptsPerSecond >= 1UL, "prescaler too coarse for a 1 Hz tick");
static_assert(kInterruptsPerSecond <= 255UL, "software divider does not fit in a byte");

// The ISR is a free function at global scope and cannot be handed a this
// pointer, so the listener lives here.
timer::ITimerListener* g_listener = 0;

// Shared between the ISR and start(). One byte, so an 8-bit part cannot tear
// it and no interrupt masking is needed around the write.
volatile uint8_t g_remaining = static_cast<uint8_t>(kInterruptsPerSecond);

}  // namespace

namespace timer {

void TimerHw::init(ITimerListener* listener) {
    g_listener = listener;

    // WGM01 alone selects CTC with OCR0A as top. No PWM on either output
    // compare unit, so D5 and D6 stay ordinary pins.
    TCCR0A = static_cast<uint8_t>(_BV(WGM01));

    // CS02 and CS00 together are the /1024 prescaler. This starts the counter,
    // but nothing is delivered until start() unmasks the interrupt.
    TCCR0B = static_cast<uint8_t>(_BV(CS02) | _BV(CS00));

    OCR0A = static_cast<uint8_t>(kCompareValue);
    TCNT0 = 0;
}

void TimerHw::start() {
    // Reset the divider so the first tick is a full second away, however long
    // the counter has been running since init().
    g_remaining = static_cast<uint8_t>(kInterruptsPerSecond);
    TCNT0 = 0;
    TIMSK0 = static_cast<uint8_t>(TIMSK0 | _BV(OCIE0A));
}

void TimerHw::stop() { TIMSK0 = static_cast<uint8_t>(TIMSK0 & ~_BV(OCIE0A)); }

}  // namespace timer

// 124 out of every 125 entries are a decrement and a branch; the listener runs
// once a second.
ISR(TIMER0_COMPA_vect) {
    if (--g_remaining != 0) {
        return;
    }

    g_remaining = static_cast<uint8_t>(kInterruptsPerSecond);
    if (g_listener != 0) {
        g_listener->onTick();
    }
}

#endif
