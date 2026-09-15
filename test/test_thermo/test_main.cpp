#include <unity.h>

#ifdef __AVR__
#include "usart/usart_c.h"
#endif

#include "Thermometer.h"
#include "mock_display.h"
#include "mock_led.h"
#include "mock_temp.h"
#include "mock_timer.h"

using display::MockDisplay;
using led::MockLed;
using temp::MockTemperature;
using timer::MockTimer;
using thermo::Thermometer;

// Table counts: 384 is 14.8 C, 462 interpolates to 21.0 C, 548 to 28.0 C.
static const uint16_t kBelow = 384;
static const uint16_t kTwentyOne = 462;
static const uint16_t kTwentyEight = 548;

void setUp(void) {}
void tearDown(void) {}

static uint8_t litCount(const MockLed& leds) {
    uint8_t n = 0;
    for (uint8_t i = 1; i <= 8; ++i) {
        if (leds.get(i)) {
            ++n;
        }
    }
    return n;
}

static void aSecondPasses(Thermometer& unit) {
    unit.onTick();
    unit.service();
}

static void inits_the_drivers(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    Thermometer unit(sensor, leds, display);

    unit.init();

    TEST_ASSERT_TRUE(leds.inited());
    TEST_ASSERT_TRUE(display.inited());
}

static void the_tick_only_starts_a_conversion(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    Thermometer unit(sensor, leds, display);
    unit.init();

    sensor.setSample(kTwentyEight);
    unit.onTick();

    TEST_ASSERT_EQUAL_UINT16(1, sensor.starts());
    TEST_ASSERT_EQUAL_UINT8(0, litCount(leds));
    TEST_ASSERT_FALSE(display.flushed());
}

static void below_twenty_one_leaves_the_bar_empty(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    Thermometer unit(sensor, leds, display);
    unit.init();

    sensor.setSample(kBelow);
    aSecondPasses(unit);

    TEST_ASSERT_EQUAL_INT16(148, unit.temperature().raw());
    TEST_ASSERT_EQUAL_UINT8(0, litCount(leds));
    TEST_ASSERT_EQUAL_UINT16(148, display.shown());
}

static void twenty_one_lights_the_first_cell(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    Thermometer unit(sensor, leds, display);
    unit.init();

    sensor.setSample(kTwentyOne);
    aSecondPasses(unit);

    TEST_ASSERT_EQUAL_INT16(210, unit.temperature().raw());
    TEST_ASSERT_EQUAL_UINT8(1, litCount(leds));
    TEST_ASSERT_TRUE(leds.get(1));
    TEST_ASSERT_EQUAL_UINT16(210, display.shown());
}

static void twenty_eight_fills_the_bar(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    Thermometer unit(sensor, leds, display);
    unit.init();

    sensor.setSample(kTwentyEight);
    aSecondPasses(unit);

    TEST_ASSERT_EQUAL_INT16(280, unit.temperature().raw());
    TEST_ASSERT_EQUAL_UINT8(8, litCount(leds));
    TEST_ASSERT_EQUAL_UINT16(280, display.shown());
}

static void a_fault_does_not_move_the_bar(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    Thermometer unit(sensor, leds, display);
    unit.init();

    sensor.setSample(kTwentyEight);
    aSecondPasses(unit);

    sensor.setSample(0);
    aSecondPasses(unit);

    TEST_ASSERT_EQUAL_UINT8(8, litCount(leds));
    TEST_ASSERT_EQUAL_INT16(280, unit.temperature().raw());
    TEST_ASSERT_EQUAL_UINT16(280, display.shown());
}

static void a_timer_tick_samples_and_draws(void) {
    MockTemperature sensor;
    MockLed leds;
    MockDisplay display;
    MockTimer clock;
    Thermometer unit(sensor, leds, display);
    unit.init();

    clock.init(&unit);
    clock.start();

    sensor.setSample(kTwentyOne);
    clock.fire(1);
    unit.service();

    TEST_ASSERT_EQUAL_UINT16(1, sensor.starts());
    TEST_ASSERT_EQUAL_UINT8(1, litCount(leds));
    TEST_ASSERT_EQUAL_UINT16(210, display.shown());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(inits_the_drivers);
    RUN_TEST(the_tick_only_starts_a_conversion);
    RUN_TEST(below_twenty_one_leaves_the_bar_empty);
    RUN_TEST(twenty_one_lights_the_first_cell);
    RUN_TEST(twenty_eight_fills_the_bar);
    RUN_TEST(a_fault_does_not_move_the_bar);
    RUN_TEST(a_timer_tick_samples_and_draws);
    return UNITY_END();
}
