#include "Solar.h"

namespace solar {

namespace {

const uint8_t kDegreesMax = 60;

const uint16_t kValveClosed = 0;
const uint16_t kValveOpen = 120;

const uint8_t kLedTlow = 1;
const uint8_t kLedThigh = 2;
const uint8_t kLedTemp = 3;
const uint8_t kLedValve = 6;
const uint8_t kLedPump = 7;

}  // namespace

Solar::Solar(temp::ITemperature& sensor, led::ILed& leds, display::IDisplay& display,
             key::IKey& keys, encoder::IEncoder& encoder, servo::IServo& valve,
             settings::ISettings& store, relay::IRelay& pump)
    : sensor_(sensor),
      leds_(leds),
      display_(display),
      keys_(keys),
      encoder_(encoder),
      valve_(valve),
      store_(store),
      pumpOut_(pump),
      temperature_(),
      // The defaults live in the store, not here: init() reloads these from it
      // once the stored pair has been read.
      tlow_(store.tlow()),
      thigh_(store.thigh()),
      edit_(store.tlow()),
      view_(View::Temperature),
      pump_(false),
      valveOpen_(true),
      encoderDown_(false),
      heldTicks_(0),
      longPress_(false) {
    for (uint8_t i = 0; i < 3; ++i) {
        keyDown_[i] = false;
    }
}

void Solar::init() {
    leds_.init();
    display_.init();
    sensor_.init();
    keys_.init();
    encoder_.init();
    valve_.init();
    store_.init();
    pumpOut_.init();

    tlow_ = store_.tlow();
    thigh_ = store_.thigh();
    edit_ = tlow_;

    valve_.setAngle(kValveOpen);
    valveOpen_ = true;
    pump_ = false;
    pumpOut_.set(false);
    view_ = View::Temperature;
    applyModeLeds();
    leds_.set(kLedValve, true);
    leds_.set(kLedPump, false);
    show();
}

void Solar::onTick() {
    sensor_.start();
    display_.onTick();

    // Interrupt context: count seconds only. The restore itself runs in
    // service(), so nothing here touches the setpoints or the LCD.
    if (encoderDown_ && view_ != View::Temperature && heldTicks_ < 255) {
        ++heldTicks_;
    }
}

void Solar::service() {
    for (uint8_t n = 1; n <= 3; ++n) {
        const bool down = keys_.get(n);
        if (down && !keyDown_[n - 1]) {
            if (n == 1) {
                enter(View::Tlow);
            } else if (n == 2) {
                enter(View::Thigh);
            } else {
                enter(View::Temperature);
            }
        }
        keyDown_[n - 1] = down;
    }

    const bool button = encoder_.button();
    onEncoderButton(button);
    encoderDown_ = button;

    // A hold cancels once the tick counter proves a full second has passed.
    // Two ticks, because the first may arrive an instant after the press.
    // The edit is abandoned and row 1 goes back to the temperature; entering
    // the view again reloads the stored setpoint.
    if (button && view_ != View::Temperature && heldTicks_ >= 2 && !longPress_) {
        longPress_ = true;
        enter(View::Temperature);
    }

    if (view_ != View::Temperature) {
        const int16_t delta = encoder_.getDelta();
        if (delta != 0) {
            edit_ = clampDegrees(static_cast<int16_t>(static_cast<int16_t>(edit_) + delta));
            show();
        }
    } else {
        (void)encoder_.getDelta();
    }

    if (sensor_.available()) {
        const temp::Reading reading = sensor_.get();
        if (reading.status == temp::SensorStatus::Ok) {
            temperature_ = reading.value;
            applyControl();
            if (view_ == View::Temperature) {
                show();
            }
        }
    }

    display_.service();
}

void Solar::applyControl() {
    const int16_t t = temperature_.raw();
    const int16_t lo = static_cast<int16_t>(static_cast<int16_t>(tlow_) * 10);
    const int16_t hi = static_cast<int16_t>(static_cast<int16_t>(thigh_) * 10);

    if (t < lo) {
        pump_ = false;
        valveOpen_ = true;
    } else if (t > hi) {
        valveOpen_ = false;
        pump_ = true;
    }

    valve_.setAngle(valveOpen_ ? kValveOpen : kValveClosed);
    pumpOut_.set(pump_);
    leds_.set(kLedValve, valveOpen_);
    leds_.set(kLedPump, pump_);
}

void Solar::applyModeLeds() {
    leds_.set(kLedTlow, view_ == View::Tlow);
    leds_.set(kLedThigh, view_ == View::Thigh);
    leds_.set(kLedTemp, view_ == View::Temperature);
}

void Solar::show() {
    if (view_ == View::Temperature) {
        display_.setHeading("TEMP");
        uint16_t deg = 0;
        if (temperature_.raw() > 0) {
            deg = static_cast<uint16_t>(temperature_.raw() / 10);
        }
        display_.showUInt(deg);
    } else if (view_ == View::Tlow) {
        display_.setHeading("TLOW");
        display_.showUInt(edit_);
    } else {
        display_.setHeading("THIGH");
        display_.showUInt(edit_);
    }
}

void Solar::enter(View view) {
    view_ = view;
    if (view == View::Tlow) {
        edit_ = tlow_;
    } else if (view == View::Thigh) {
        edit_ = thigh_;
    }
    heldTicks_ = 0;
    longPress_ = false;
    applyModeLeds();
    show();
}

void Solar::onEncoderButton(bool down) {
    if (view_ == View::Temperature) {
        return;
    }

    if (down && !encoderDown_) {
        heldTicks_ = 0;
        longPress_ = false;
        return;
    }

    if (!down && encoderDown_ && !longPress_) {
        // Main-loop context, and only on the commit: the write blocks for
        // milliseconds and spends one of the cell's finite erase cycles.
        if (view_ == View::Tlow) {
            store_.save(edit_, thigh_);
        } else {
            store_.save(tlow_, edit_);
        }
        // The store cancels a pair it cannot hold, so it is the authority on
        // what was kept rather than the edit that was offered.
        tlow_ = store_.tlow();
        thigh_ = store_.thigh();
        enter(View::Temperature);
    }
}

uint8_t Solar::clampDegrees(int16_t value) const {
    if (value < 0) {
        return 0;
    }
    if (value > static_cast<int16_t>(kDegreesMax)) {
        return kDegreesMax;
    }
    return static_cast<uint8_t>(value);
}

temp::DeciCelsius Solar::temperature() const { return temperature_; }

uint8_t Solar::tlow() const { return tlow_; }

uint8_t Solar::thigh() const { return thigh_; }

bool Solar::pump() const { return pump_; }

bool Solar::valveOpen() const { return valveOpen_; }

}  // namespace solar
