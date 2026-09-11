#include "temp/mock_temp.h"

#include "temp/Conversion.h"

namespace temp {

MockTemperature::MockTemperature()
    : sample_(512), latestRaw_(512), starts_(0), ready_(false) {}

void MockTemperature::init() {}

void MockTemperature::start() {
    ++starts_;
    latestRaw_ = sample_;
    ready_ = true;
}

bool MockTemperature::available() {
    const bool ready = ready_;
    ready_ = false;
    return ready;
}

uint16_t MockTemperature::readRaw() const { return latestRaw_; }

Reading MockTemperature::celsius(uint16_t raw) const { return convert(raw); }

Reading MockTemperature::get() const { return convert(latestRaw_); }

void MockTemperature::setSample(uint16_t raw) { sample_ = raw; }

uint16_t MockTemperature::starts() const { return starts_; }

}  // namespace temp
