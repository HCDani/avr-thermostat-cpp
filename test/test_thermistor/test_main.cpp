#include <unity.h>

#include "thermostat/Thermistor.h"

using thermostat::AdcCount;
using thermostat::DeciCelsius;
using thermostat::Thermistor;

void setUp(void) {}
void tearDown(void) {}

// Reference values come from tools/gen_thermistor_table.py, which prints them.
// These counts are all exact breakpoints, so no interpolation is involved and
// the comparison can be exact.
static void midpoint_of_the_divider_is_twenty_five_degrees(void) {
    TEST_ASSERT_EQUAL_INT16(250, Thermistor::convert(AdcCount(512)).raw());
}

static void known_breakpoints_match_the_reference_curve(void) {
    TEST_ASSERT_EQUAL_INT16(38, Thermistor::convert(AdcCount(256)).raw());
    TEST_ASSERT_EQUAL_INT16(148, Thermistor::convert(AdcCount(384)).raw());
    TEST_ASSERT_EQUAL_INT16(361, Thermistor::convert(AdcCount(640)).raw());
    TEST_ASSERT_EQUAL_INT16(498, Thermistor::convert(AdcCount(768)).raw());
}

static void both_rails_are_rejected(void) {
    TEST_ASSERT_FALSE(Thermistor::inRange(AdcCount(0)));
    TEST_ASSERT_FALSE(Thermistor::inRange(AdcCount(1023)));
}

static void valid_window_edges_are_accepted(void) {
    TEST_ASSERT_TRUE(Thermistor::inRange(AdcCount(Thermistor::minValidCount())));
    TEST_ASSERT_TRUE(Thermistor::inRange(AdcCount(Thermistor::maxValidCount())));
    TEST_ASSERT_FALSE(Thermistor::inRange(AdcCount(Thermistor::minValidCount() - 1)));
    TEST_ASSERT_FALSE(Thermistor::inRange(AdcCount(Thermistor::maxValidCount() + 1)));
}

// A higher count means less resistance, which for an NTC means a warmer
// thermistor. If this ever fails the divider has been wired the other way up
// and every control decision is inverted.
static void curve_rises_monotonically_across_the_valid_window(void) {
    int16_t previous = Thermistor::convert(AdcCount(Thermistor::minValidCount())).raw();
    for (uint16_t count = Thermistor::minValidCount() + 1; count <= Thermistor::maxValidCount();
         ++count) {
        const int16_t current = Thermistor::convert(AdcCount(count)).raw();
        TEST_ASSERT_GREATER_OR_EQUAL_INT16(previous, current);
        previous = current;
    }
}

static void interpolated_values_sit_between_their_breakpoints(void) {
    const int16_t lower = Thermistor::convert(AdcCount(512)).raw();
    const int16_t upper = Thermistor::convert(AdcCount(520)).raw();
    for (uint16_t count = 513; count < 520; ++count) {
        const int16_t value = Thermistor::convert(AdcCount(count)).raw();
        TEST_ASSERT_GREATER_OR_EQUAL_INT16(lower, value);
        TEST_ASSERT_LESS_OR_EQUAL_INT16(upper, value);
    }
}

// Reading one past the last breakpoint must not walk off the table.
static void top_of_the_range_does_not_read_past_the_table(void) {
    const int16_t last = Thermistor::convert(AdcCount(1023)).raw();
    TEST_ASSERT_GREATER_THAN_INT16(0, last);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(midpoint_of_the_divider_is_twenty_five_degrees);
    RUN_TEST(known_breakpoints_match_the_reference_curve);
    RUN_TEST(both_rails_are_rejected);
    RUN_TEST(valid_window_edges_are_accepted);
    RUN_TEST(curve_rises_monotonically_across_the_valid_window);
    RUN_TEST(interpolated_values_sit_between_their_breakpoints);
    RUN_TEST(top_of_the_range_does_not_read_past_the_table);
    return UNITY_END();
}
