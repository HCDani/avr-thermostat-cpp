#include <unity.h>

#ifdef __AVR__
#include "usart/usart_c.h"
#endif

#include "Conversion.h"
#include "ThermistorTable.h"

using temp::DeciCelsius;
using temp::Reading;
using temp::SensorStatus;
using temp::convert;

void setUp(void) {}
void tearDown(void) {}

// Reference values come from tools/gen_thermistor_table.py, which prints them.
// These counts are all exact breakpoints, so no interpolation is involved and
// the comparison can be exact.
static void midpoint_of_the_divider_is_twenty_five_degrees(void) {
    TEST_ASSERT_EQUAL_INT16(250, convert(512).value.raw());
}

static void known_breakpoints_match_the_reference_curve(void) {
    TEST_ASSERT_EQUAL_INT16(38, convert(256).value.raw());
    TEST_ASSERT_EQUAL_INT16(148, convert(384).value.raw());
    TEST_ASSERT_EQUAL_INT16(361, convert(640).value.raw());
    TEST_ASSERT_EQUAL_INT16(498, convert(768).value.raw());
}

static void both_rails_are_rejected(void) {
    TEST_ASSERT_TRUE(convert(0).status == SensorStatus::Open);
    TEST_ASSERT_TRUE(convert(1023).status == SensorStatus::Shorted);
}

static void valid_window_edges_are_accepted(void) {
    TEST_ASSERT_TRUE(convert(temp::table::kMinValidCount).status == SensorStatus::Ok);
    TEST_ASSERT_TRUE(convert(temp::table::kMaxValidCount).status == SensorStatus::Ok);
    TEST_ASSERT_TRUE(convert(temp::table::kMinValidCount - 1).status == SensorStatus::Open);
    TEST_ASSERT_TRUE(convert(temp::table::kMaxValidCount + 1).status == SensorStatus::Shorted);
}

static void interpolated_values_sit_between_their_breakpoints(void) {
    const int16_t lower = convert(512).value.raw();
    const int16_t upper = convert(520).value.raw();
    for (uint16_t count = 513; count < 520; ++count) {
        const int16_t value = convert(count).value.raw();
        TEST_ASSERT_GREATER_OR_EQUAL_INT16(lower, value);
        TEST_ASSERT_LESS_OR_EQUAL_INT16(upper, value);
    }
}

// Reading one past the last breakpoint must not walk off the table.
static void top_of_the_range_does_not_read_past_the_table(void) {
    const int16_t last = convert(1023).value.raw();
    TEST_ASSERT_EQUAL_INT16(0, last);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(midpoint_of_the_divider_is_twenty_five_degrees);
    RUN_TEST(known_breakpoints_match_the_reference_curve);
    RUN_TEST(both_rails_are_rejected);
    RUN_TEST(valid_window_edges_are_accepted);
    RUN_TEST(interpolated_values_sit_between_their_breakpoints);
    RUN_TEST(top_of_the_range_does_not_read_past_the_table);
    return UNITY_END();
}
