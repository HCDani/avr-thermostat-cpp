#include "mock_settings.h"

#include "Sanitize.h"

namespace settings {

namespace {

const uint8_t kBlank = 0xFF;

}  // namespace

MockSettings::MockSettings()
    : storedTlow_(kBlank), storedThigh_(kBlank), tlow_(kBlank), thigh_(kBlank), writes_(0), inited_(false) {
    sanitize(tlow_, thigh_);
}

void MockSettings::init() {
    uint8_t tlow = storedTlow_;
    uint8_t thigh = storedThigh_;
    sanitize(tlow, thigh);
    tlow_ = tlow;
    thigh_ = thigh;
    inited_ = true;
}

uint8_t MockSettings::tlow() const { return tlow_; }

uint8_t MockSettings::thigh() const { return thigh_; }

void MockSettings::save(uint8_t tlow, uint8_t thigh) {
    if (!usable(tlow, thigh)) {
        return;
    }
    if (tlow != storedTlow_) {
        ++writes_;
    }
    if (thigh != storedThigh_) {
        ++writes_;
    }
    storedTlow_ = tlow;
    storedThigh_ = thigh;
    tlow_ = tlow;
    thigh_ = thigh;
}

void MockSettings::setStored(uint8_t tlow, uint8_t thigh) {
    storedTlow_ = tlow;
    storedThigh_ = thigh;
}

uint8_t MockSettings::storedTlow() const { return storedTlow_; }

uint8_t MockSettings::storedThigh() const { return storedThigh_; }

uint16_t MockSettings::writes() const { return writes_; }

bool MockSettings::inited() const { return inited_; }

}  // namespace settings
