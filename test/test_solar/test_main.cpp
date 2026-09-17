#include <unity.h>

#ifdef __AVR__
#include "usart/usart_c.h"
#endif

#include "Solar.h"
#include "mock_display.h"
#include "mock_encoder.h"
#include "mock_key.h"
#include "mock_led.h"
#include "mock_servo.h"
#include "mock_settings.h"
#include "mock_relay.h"
#include "mock_temp.h"

using display::MockDisplay;
using encoder::MockEncoder;
using key::MockKey;
using led::MockLed;
using relay::MockRelay;
using servo::MockServo;
using settings::MockSettings;
using solar::Solar;
using temp::MockTemperature;

// Table counts: 384 is 14.8 C, 512 is 25.0 C, 528 is 26.3 C.
static const uint16_t kCold = 384;
static const uint16_t kBand = 512;
static const uint16_t kHot = 528;

void setUp(void) {}
void tearDown(void) {}

static void aSecondPasses(Solar& unit) {
    unit.onTick();
    unit.service();
}

static void inits_the_drivers(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);

    unit.init();

    TEST_ASSERT_TRUE(leds.inited());
    TEST_ASSERT_TRUE(display.inited());
    TEST_ASSERT_TRUE(keys.inited());
    TEST_ASSERT_TRUE(encoder.inited());
    TEST_ASSERT_TRUE(valve.inited());
    TEST_ASSERT_TRUE(store.inited());
    TEST_ASSERT_TRUE(pump.inited());
    TEST_ASSERT_FALSE(pump.get());
    TEST_ASSERT_TRUE(unit.valveOpen());
    TEST_ASSERT_FALSE(unit.pump());
    TEST_ASSERT_EQUAL_UINT16(120, valve.angle());
    TEST_ASSERT_TRUE(leds.get(3));
    TEST_ASSERT_FALSE(leds.get(1));
    TEST_ASSERT_FALSE(leds.get(2));
}

static void below_tlow_opens_the_valve_and_stops_the_pump(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    sensor.setSample(kCold);
    aSecondPasses(unit);

    TEST_ASSERT_TRUE(unit.valveOpen());
    TEST_ASSERT_FALSE(unit.pump());
    TEST_ASSERT_FALSE(pump.get());
    TEST_ASSERT_TRUE(leds.get(6));
    TEST_ASSERT_FALSE(leds.get(7));
    TEST_ASSERT_EQUAL_UINT16(120, valve.angle());
}

static void above_thigh_closes_the_valve_and_starts_the_pump(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    sensor.setSample(kHot);
    aSecondPasses(unit);

    TEST_ASSERT_FALSE(unit.valveOpen());
    TEST_ASSERT_TRUE(unit.pump());
    TEST_ASSERT_TRUE(pump.get());
    TEST_ASSERT_FALSE(leds.get(6));
    TEST_ASSERT_TRUE(leds.get(7));
    TEST_ASSERT_EQUAL_UINT16(0, valve.angle());
    TEST_ASSERT_EQUAL_STRING("TEMP", display.heading());
    TEST_ASSERT_EQUAL_UINT16(26, display.shown());
}

static void the_band_keeps_the_previous_state(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    sensor.setSample(kHot);
    aSecondPasses(unit);
    sensor.setSample(kBand);
    aSecondPasses(unit);

    TEST_ASSERT_FALSE(unit.valveOpen());
    TEST_ASSERT_TRUE(unit.pump());
}

static void key1_shows_tlow(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    keys.press(1, true);
    unit.service();

    TEST_ASSERT_EQUAL_STRING("TLOW", display.heading());
    TEST_ASSERT_EQUAL_UINT16(18, display.shown());
    TEST_ASSERT_TRUE(leds.get(1));
    TEST_ASSERT_FALSE(leds.get(2));
    TEST_ASSERT_FALSE(leds.get(3));
}

static void key2_shows_thigh(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    keys.press(2, true);
    unit.service();

    TEST_ASSERT_EQUAL_STRING("THIGH", display.heading());
    TEST_ASSERT_EQUAL_UINT16(25, display.shown());
    TEST_ASSERT_TRUE(leds.get(2));
}

static void key3_returns_to_temperature(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    keys.press(1, true);
    unit.service();
    keys.press(1, false);
    keys.press(3, true);
    unit.service();

    TEST_ASSERT_EQUAL_STRING("TEMP", display.heading());
    TEST_ASSERT_TRUE(leds.get(3));
}

static void encoder_adjusts_tlow_and_a_short_press_saves(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    keys.press(1, true);
    unit.service();
    keys.press(1, false);

    encoder.turn(2);
    unit.service();

    TEST_ASSERT_EQUAL_UINT16(20, display.shown());
    TEST_ASSERT_EQUAL_UINT8(18, unit.tlow());

    encoder.press(true);
    unit.service();
    encoder.press(false);
    unit.service();

    TEST_ASSERT_EQUAL_UINT8(20, unit.tlow());
    TEST_ASSERT_EQUAL_STRING("TEMP", display.heading());
    TEST_ASSERT_TRUE(leds.get(3));
    TEST_ASSERT_FALSE(leds.get(1));
}

static void a_long_press_cancels_the_edit(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    keys.press(2, true);
    unit.service();
    keys.press(2, false);

    encoder.turn(-5);
    unit.service();
    TEST_ASSERT_EQUAL_UINT16(20, display.shown());

    encoder.press(true);
    unit.service();
    unit.onTick();
    unit.onTick();
    unit.service();
    encoder.press(false);
    unit.service();

    TEST_ASSERT_EQUAL_UINT8(25, unit.thigh());
    TEST_ASSERT_EQUAL_STRING("TEMP", display.heading());
    TEST_ASSERT_TRUE(leds.get(3));
    TEST_ASSERT_FALSE(leds.get(2));
}

// Abandoning an edit must not leave it behind: entering the view again shows
// the stored setpoint, not what was dialled in and cancelled.
static void a_cancelled_edit_is_not_remembered(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    keys.press(1, true);
    unit.service();
    keys.press(1, false);
    unit.service();

    encoder.turn(4);
    unit.service();

    encoder.press(true);
    unit.service();
    unit.onTick();
    unit.onTick();
    unit.service();
    encoder.press(false);
    unit.service();

    keys.press(1, true);
    unit.service();

    TEST_ASSERT_EQUAL_UINT16(18, display.shown());
}

// One tick may land an instant after the press, so a single tick is not proof
// of a full second and must still count as a short press.
static void a_press_across_one_tick_still_saves(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    keys.press(1, true);
    unit.service();
    keys.press(1, false);

    encoder.turn(3);
    unit.service();

    encoder.press(true);
    unit.service();
    unit.onTick();
    unit.service();
    encoder.press(false);
    unit.service();

    TEST_ASSERT_EQUAL_UINT8(21, unit.tlow());
    TEST_ASSERT_EQUAL_STRING("TEMP", display.heading());
}

static void encoder_is_ignored_in_temperature_view(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    encoder.turn(4);
    unit.service();

    TEST_ASSERT_EQUAL_UINT8(18, unit.tlow());
    TEST_ASSERT_EQUAL_UINT8(25, unit.thigh());
}

static void edits_are_clamped_to_zero_and_sixty(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    keys.press(1, true);
    unit.service();

    encoder.turn(-20);
    unit.service();
    TEST_ASSERT_EQUAL_UINT16(0, display.shown());

    encoder.turn(80);
    unit.service();
    TEST_ASSERT_EQUAL_UINT16(60, display.shown());
}

// The setpoints come out of the store, so a board that has run before starts
// where it left off rather than at the defaults.
static void stored_setpoints_survive_a_restart(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    store.setStored(10, 30);
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);

    unit.init();

    TEST_ASSERT_EQUAL_UINT8(10, unit.tlow());
    TEST_ASSERT_EQUAL_UINT8(30, unit.thigh());
}

static void a_commit_reaches_the_store(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    keys.press(1, true);
    unit.service();
    keys.press(1, false);
    encoder.turn(3);
    unit.service();
    encoder.press(true);
    unit.service();
    encoder.press(false);
    unit.service();

    TEST_ASSERT_EQUAL_UINT8(21, store.storedTlow());
    TEST_ASSERT_EQUAL_UINT8(25, store.storedThigh());
}

// A cancelled edit must not spend an erase cycle, and neither must a tick:
// at 1 Hz a write per tick would wear a cell out in about a day.
static void a_cancelled_edit_writes_nothing(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    keys.press(1, true);
    unit.service();
    keys.press(1, false);
    encoder.turn(4);
    unit.service();
    encoder.press(true);
    unit.service();
    unit.onTick();
    unit.onTick();
    unit.service();
    encoder.press(false);
    unit.service();

    TEST_ASSERT_EQUAL_UINT16(0, store.writes());
}

static void running_never_writes(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    sensor.setSample(kHot);
    for (uint8_t i = 0; i < 10; ++i) {
        aSecondPasses(unit);
    }

    TEST_ASSERT_EQUAL_UINT16(0, store.writes());
}

// The store owns the pair rule, so a tlow dialled above thigh is cancelled
// there and both setpoints keep the values they had. Stored pair rather than
// the defaults, so a substitution would be visible if one happened.
static void an_out_of_order_commit_is_cancelled(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockKey keys;
    MockEncoder encoder;
    MockServo valve;
    MockSettings store;
    store.setStored(10, 30);
    MockRelay pump;
    Solar unit(sensor, leds, display, keys, encoder, valve, store, pump);
    unit.init();

    keys.press(1, true);
    unit.service();
    keys.press(1, false);
    encoder.turn(30);
    unit.service();
    TEST_ASSERT_EQUAL_UINT16(40, display.shown());

    encoder.press(true);
    unit.service();
    encoder.press(false);
    unit.service();

    TEST_ASSERT_EQUAL_UINT8(10, unit.tlow());
    TEST_ASSERT_EQUAL_UINT8(30, unit.thigh());
    TEST_ASSERT_EQUAL_UINT8(10, store.storedTlow());
    TEST_ASSERT_EQUAL_UINT8(30, store.storedThigh());
    TEST_ASSERT_EQUAL_UINT16(0, store.writes());
    TEST_ASSERT_EQUAL_STRING("TEMP", display.heading());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(inits_the_drivers);
    RUN_TEST(below_tlow_opens_the_valve_and_stops_the_pump);
    RUN_TEST(above_thigh_closes_the_valve_and_starts_the_pump);
    RUN_TEST(the_band_keeps_the_previous_state);
    RUN_TEST(key1_shows_tlow);
    RUN_TEST(key2_shows_thigh);
    RUN_TEST(key3_returns_to_temperature);
    RUN_TEST(encoder_adjusts_tlow_and_a_short_press_saves);
    RUN_TEST(a_long_press_cancels_the_edit);
    RUN_TEST(a_cancelled_edit_is_not_remembered);
    RUN_TEST(a_press_across_one_tick_still_saves);
    RUN_TEST(encoder_is_ignored_in_temperature_view);
    RUN_TEST(edits_are_clamped_to_zero_and_sixty);
    RUN_TEST(stored_setpoints_survive_a_restart);
    RUN_TEST(a_commit_reaches_the_store);
    RUN_TEST(a_cancelled_edit_writes_nothing);
    RUN_TEST(running_never_writes);
    RUN_TEST(an_out_of_order_commit_is_cancelled);
    return UNITY_END();
}
