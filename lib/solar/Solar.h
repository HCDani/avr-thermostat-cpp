#pragma once

#include "IDisplay.h"
#include "IEncoder.h"
#include "IKey.h"
#include "ILed.h"
#include "IRelay.h"
#include "IServo.h"
#include "ISettings.h"
#include "ITemperature.h"
#include "ITimer.h"
#include "DeciCelsius.h"

namespace solar {

// Part 4: panel temperature drives a valve (servo) and a pump (row-2 pos 7)
// with hysteresis between tlow and thigh. Keys select TEMP / TLOW / THIGH
// on row 1; the encoder edits a setpoint while that view is showing, and a
// short press commits it through ISettings so it survives a power cycle.
class Solar : public timer::ITimerListener {
public:
    Solar(temp::ITemperature& sensor, led::ILed& leds, display::IDisplay& display,
          key::IKey& keys, encoder::IEncoder& encoder, servo::IServo& valve,
          settings::ISettings& store, relay::IRelay& pump);

    void init();
    void onTick() override;
    void service();

    temp::DeciCelsius temperature() const;
    uint8_t tlow() const;
    uint8_t thigh() const;
    bool pump() const;
    bool valveOpen() const;

private:
    enum class View : uint8_t { Temperature, Tlow, Thigh };

    void applyControl();
    void applyModeLeds();
    void show();
    void enter(View view);
    void onEncoderButton(bool down);
    uint8_t clampDegrees(int16_t value) const;

    temp::ITemperature& sensor_;
    led::ILed& leds_;
    display::IDisplay& display_;
    key::IKey& keys_;
    encoder::IEncoder& encoder_;
    servo::IServo& valve_;
    settings::ISettings& store_;
    relay::IRelay& pumpOut_;

    temp::DeciCelsius temperature_;
    uint8_t tlow_;
    uint8_t thigh_;
    uint8_t edit_;
    View view_;
    bool pump_;
    bool valveOpen_;
    bool keyDown_[3];
    bool encoderDown_;
    // Counted in the tick ISR, read in service(), so it must not be cached.
    volatile uint8_t heldTicks_;
    bool longPress_;
};

}  // namespace solar
