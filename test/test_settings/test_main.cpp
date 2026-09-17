#include <unity.h>

#ifdef __AVR__
#include "usart/usart_c.h"
#endif

#include "Sanitize.h"
#include "mock_settings.h"

using settings::MockSettings;
using settings::sanitize;

void setUp(void) {}
void tearDown(void) {}

// The defaults and the 0..60 C range are the specification's (6.4), and the
// range is the same one the encoder enforces while editing (6.2).
static void a_blank_eeprom_reads_as_the_defaults(void) {
    MockSettings unit;

    unit.init();

    TEST_ASSERT_TRUE(unit.inited());
    TEST_ASSERT_EQUAL_UINT8(18, unit.tlow());
    TEST_ASSERT_EQUAL_UINT8(25, unit.thigh());
}

static void a_stored_pair_survives_the_read(void) {
    MockSettings unit;
    unit.setStored(12, 40);

    unit.init();

    TEST_ASSERT_EQUAL_UINT8(12, unit.tlow());
    TEST_ASSERT_EQUAL_UINT8(40, unit.thigh());
}

static void the_range_edges_are_accepted(void) {
    MockSettings unit;
    unit.setStored(0, 60);

    unit.init();

    TEST_ASSERT_EQUAL_UINT8(0, unit.tlow());
    TEST_ASSERT_EQUAL_UINT8(60, unit.thigh());
}

// The read has to produce something to run with, so it substitutes the
// defaults. A save has a good stored pair behind it and cancels instead.
static void a_value_above_the_range_falls_back_to_both_defaults(void) {
    uint8_t tlow = 18;
    uint8_t thigh = 61;

    sanitize(tlow, thigh);

    TEST_ASSERT_EQUAL_UINT8(18, tlow);
    TEST_ASSERT_EQUAL_UINT8(25, thigh);
}

static void an_out_of_order_pair_falls_back_to_the_defaults(void) {
    uint8_t tlow = 40;
    uint8_t thigh = 12;

    sanitize(tlow, thigh);

    TEST_ASSERT_EQUAL_UINT8(18, tlow);
    TEST_ASSERT_EQUAL_UINT8(25, thigh);
}

static void equal_setpoints_are_rejected(void) {
    uint8_t tlow = 20;
    uint8_t thigh = 20;

    sanitize(tlow, thigh);

    TEST_ASSERT_EQUAL_UINT8(18, tlow);
    TEST_ASSERT_EQUAL_UINT8(25, thigh);
}

static void a_saved_pair_is_readable_without_a_reread(void) {
    MockSettings unit;
    unit.init();

    unit.save(10, 30);

    TEST_ASSERT_EQUAL_UINT8(10, unit.tlow());
    TEST_ASSERT_EQUAL_UINT8(30, unit.thigh());
}

static void a_saved_pair_survives_a_restart(void) {
    MockSettings unit;
    unit.init();
    unit.save(10, 30);

    unit.init();

    TEST_ASSERT_EQUAL_UINT8(10, unit.tlow());
    TEST_ASSERT_EQUAL_UINT8(30, unit.thigh());
}

static void saving_an_out_of_order_pair_changes_nothing(void) {
    MockSettings unit;
    unit.setStored(10, 30);
    unit.init();

    unit.save(40, 30);

    TEST_ASSERT_EQUAL_UINT8(10, unit.tlow());
    TEST_ASSERT_EQUAL_UINT8(30, unit.thigh());
    TEST_ASSERT_EQUAL_UINT8(10, unit.storedTlow());
    TEST_ASSERT_EQUAL_UINT8(30, unit.storedThigh());
    TEST_ASSERT_EQUAL_UINT16(0, unit.writes());
}

static void saving_a_value_above_the_range_changes_nothing(void) {
    MockSettings unit;
    unit.setStored(10, 30);
    unit.init();

    unit.save(10, 61);

    TEST_ASSERT_EQUAL_UINT8(10, unit.tlow());
    TEST_ASSERT_EQUAL_UINT8(30, unit.thigh());
    TEST_ASSERT_EQUAL_UINT16(0, unit.writes());
}

static void saving_an_unchanged_value_costs_no_erase_cycle(void) {
    MockSettings unit;
    unit.init();
    unit.save(10, 30);
    const uint16_t spent = unit.writes();

    unit.save(10, 30);

    TEST_ASSERT_EQUAL_UINT16(spent, unit.writes());
}

static void only_the_changed_byte_is_written(void) {
    MockSettings unit;
    unit.init();
    unit.save(10, 30);
    const uint16_t spent = unit.writes();

    unit.save(10, 31);

    TEST_ASSERT_EQUAL_UINT16(spent + 1, unit.writes());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(a_blank_eeprom_reads_as_the_defaults);
    RUN_TEST(a_stored_pair_survives_the_read);
    RUN_TEST(the_range_edges_are_accepted);
    RUN_TEST(a_value_above_the_range_falls_back_to_both_defaults);
    RUN_TEST(an_out_of_order_pair_falls_back_to_the_defaults);
    RUN_TEST(equal_setpoints_are_rejected);
    RUN_TEST(a_saved_pair_is_readable_without_a_reread);
    RUN_TEST(a_saved_pair_survives_a_restart);
    RUN_TEST(saving_an_out_of_order_pair_changes_nothing);
    RUN_TEST(saving_a_value_above_the_range_changes_nothing);
    RUN_TEST(saving_an_unchanged_value_costs_no_erase_cycle);
    RUN_TEST(only_the_changed_byte_is_written);
    return UNITY_END();
}
