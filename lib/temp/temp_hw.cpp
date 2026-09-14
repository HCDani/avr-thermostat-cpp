#include "temp_hw.h"

#include "Conversion.h"

#ifdef __AVR__
#include <avr/interrupt.h>
#endif

namespace temp {

TemperatureHw* TemperatureHw::active_ = 0;

TemperatureHw::TemperatureHw(adc::Adc& adc, uint8_t pin)
    : adc_(adc), pin_(pin), latestRaw_(0), ready_(false) {}

void TemperatureHw::init() { active_ = this; }

void TemperatureHw::start() {
    active_ = this;
    adc_.start(pin_, &TemperatureHw::onConversionComplete);
}

bool TemperatureHw::available() {
#ifdef __AVR__
    const uint8_t saved = SREG;
    cli();
    const bool ready = ready_;
    ready_ = false;
    SREG = saved;
    return ready;
#else
    const bool ready = ready_;
    ready_ = false;
    return ready;
#endif
}

uint16_t TemperatureHw::readRaw() const {
#ifdef __AVR__
    // Two bytes on an 8-bit part, and the ISR writes them roughly when the
    // tick asks for them, so an unguarded read can tear. Mask for the load
    // rather than return a sample that was never measured.
    const uint8_t saved = SREG;
    cli();
    const uint16_t raw = latestRaw_;
    SREG = saved;
    return raw;
#else
    return latestRaw_;
#endif
}

Reading TemperatureHw::celsius(uint16_t raw) const { return convert(raw); }

Reading TemperatureHw::get() const { return convert(readRaw()); }

// Interrupt context: latch the count and nothing else. No conversion here --
// that is the whole reason readRaw() and get() are separate.
//
// Before the first conversion completes the latch holds 0, which converts to
// Open. That is honest: there is no measurement yet, and the fault debounce
// absorbs it.
void TemperatureHw::onConversionComplete(uint16_t adcvalue) {
    if (active_ != 0) {
        active_->latestRaw_ = adcvalue;
        active_->ready_ = true;
    }
}

}  // namespace temp
