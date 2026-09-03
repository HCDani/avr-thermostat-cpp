#include <unity.h>

#include "thermostat/Controller.h"
#include "thermostat/testing/FakeHal.h"

using thermostat::AdcCount;
using thermostat::Config;
using thermostat::Controller;
using thermostat::DeciCelsius;
using thermostat::SetpointLimits;
using thermostat::testing::FakeHal;

using Thermostat = Controller<FakeHal>;
using State = Thermostat::State;

// Counts taken from the generated table: 384 is 14.8 C, 512 is 25.0 C and
// 640 is 36.1 C. With a 25.0 C setpoint and 0.5 C of hysteresis the heater
// turns on at or below 24.5 C and off at or above 25.5 C, so 512 sits inside
// the band and must not move anything on its own.
static const uint16_t kCold = 384;
static const uint16_t kInBand = 512;
static const uint16_t kHot = 640;
static const uint16_t kOpenCircuit = 0;

static Config config() {
    Config cfg;
    cfg.setpoint = DeciCelsius(250);
    cfg.hysteresis = DeciCelsius(5);
    cfg.limits = SetpointLimits{DeciCelsius(50), DeciCelsius(350)};
    cfg.faultSampleLimit = 3;
    return cfg;
}

void setUp(void) {}
void tearDown(void) {}

static void starts_idle_with_the_heater_off(void) {
    FakeHal hal;
    Thermostat unit(hal, config());

    TEST_ASSERT_EQUAL(State::Idle, unit.state());
    TEST_ASSERT_FALSE(hal.heater());
    TEST_ASSERT_EQUAL_UINT16(0, hal.reads());
}

static void heats_when_below_the_lower_bound(void) {
    FakeHal hal;
    Thermostat unit(hal, config());

    hal.setSample(kCold);
    unit.tick();

    TEST_ASSERT_EQUAL(State::Heating, unit.state());
    TEST_ASSERT_TRUE(hal.heater());
    TEST_ASSERT_EQUAL_INT16(148, unit.temperature().raw());
}

// The point of hysteresis: once heating, a reading inside the band leaves the
// relay alone. A controller that switched here would chatter around setpoint.
static void keeps_heating_inside_the_band(void) {
    FakeHal hal;
    Thermostat unit(hal, config());

    hal.setSample(kCold);
    unit.tick();
    hal.setSample(kInBand);
    unit.tick();

    TEST_ASSERT_EQUAL(State::Heating, unit.state());
    TEST_ASSERT_TRUE(hal.heater());
    TEST_ASSERT_EQUAL_UINT16(1, hal.transitions());
}

static void stops_heating_above_the_upper_bound(void) {
    FakeHal hal;
    Thermostat unit(hal, config());

    hal.setSample(kCold);
    unit.tick();
    hal.setSample(kHot);
    unit.tick();

    TEST_ASSERT_EQUAL(State::Idle, unit.state());
    TEST_ASSERT_FALSE(hal.heater());
    TEST_ASSERT_EQUAL_UINT16(2, hal.transitions());
}

static void stays_idle_inside_the_band(void) {
    FakeHal hal;
    Thermostat unit(hal, config());

    hal.setSample(kInBand);
    unit.tick();

    TEST_ASSERT_EQUAL(State::Idle, unit.state());
    TEST_ASSERT_FALSE(hal.heater());
    TEST_ASSERT_EQUAL_UINT16(0, hal.transitions());
}

// A single dropout is noise and must not cut the heat.
static void tolerates_fewer_bad_samples_than_the_limit(void) {
    FakeHal hal;
    Thermostat unit(hal, config());

    hal.setSample(kCold);
    unit.tick();

    hal.setSample(kOpenCircuit);
    unit.tick();
    unit.tick();

    TEST_ASSERT_EQUAL(State::Heating, unit.state());
    TEST_ASSERT_TRUE(hal.heater());
}

static void faults_and_cuts_the_heater_after_the_limit(void) {
    FakeHal hal;
    Thermostat unit(hal, config());

    hal.setSample(kCold);
    unit.tick();
    TEST_ASSERT_TRUE(hal.heater());

    hal.setSample(kOpenCircuit);
    unit.tick();
    unit.tick();
    unit.tick();

    TEST_ASSERT_EQUAL(State::Fault, unit.state());
    TEST_ASSERT_FALSE(hal.heater());
}

static void stays_off_while_faulted(void) {
    FakeHal hal;
    Thermostat unit(hal, config());

    hal.setSample(kOpenCircuit);
    for (int i = 0; i < 10; ++i) {
        unit.tick();
    }

    TEST_ASSERT_EQUAL(State::Fault, unit.state());
    TEST_ASSERT_FALSE(hal.heater());
}

static void recovers_once_the_sensor_reads_again(void) {
    FakeHal hal;
    Thermostat unit(hal, config());

    hal.setSample(kOpenCircuit);
    for (int i = 0; i < 5; ++i) {
        unit.tick();
    }
    TEST_ASSERT_EQUAL(State::Fault, unit.state());

    hal.setSample(kCold);
    unit.tick();

    TEST_ASSERT_EQUAL(State::Heating, unit.state());
    TEST_ASSERT_TRUE(hal.heater());
}

// The bad-sample counter has to reset, or a dropout every other tick would
// eventually fault a perfectly healthy sensor.
static void intermittent_dropouts_never_reach_the_limit(void) {
    FakeHal hal;
    Thermostat unit(hal, config());

    for (int i = 0; i < 20; ++i) {
        hal.setSample(kOpenCircuit);
        unit.tick();
        hal.setSample(kInBand);
        unit.tick();
    }

    TEST_ASSERT_NOT_EQUAL(State::Fault, unit.state());
}

static void setpoint_is_clamped_to_its_limits(void) {
    FakeHal hal;
    Thermostat unit(hal, config());

    unit.setSetpoint(DeciCelsius(4000));
    TEST_ASSERT_EQUAL_INT16(350, unit.setpoint().raw());

    unit.setSetpoint(DeciCelsius(-4000));
    TEST_ASSERT_EQUAL_INT16(50, unit.setpoint().raw());

    unit.setSetpoint(DeciCelsius(210));
    TEST_ASSERT_EQUAL_INT16(210, unit.setpoint().raw());
}

// A setpoint restored from a blank or corrupted EEPROM must not be acted on.
static void construction_clamps_an_out_of_range_setpoint(void) {
    FakeHal hal;
    Config cfg = config();
    cfg.setpoint = DeciCelsius(32767);
    Thermostat unit(hal, cfg);

    TEST_ASSERT_EQUAL_INT16(350, unit.setpoint().raw());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(starts_idle_with_the_heater_off);
    RUN_TEST(heats_when_below_the_lower_bound);
    RUN_TEST(keeps_heating_inside_the_band);
    RUN_TEST(stops_heating_above_the_upper_bound);
    RUN_TEST(stays_idle_inside_the_band);
    RUN_TEST(tolerates_fewer_bad_samples_than_the_limit);
    RUN_TEST(faults_and_cuts_the_heater_after_the_limit);
    RUN_TEST(stays_off_while_faulted);
    RUN_TEST(recovers_once_the_sensor_reads_again);
    RUN_TEST(intermittent_dropouts_never_reach_the_limit);
    RUN_TEST(setpoint_is_clamped_to_its_limits);
    RUN_TEST(construction_clamps_an_out_of_range_setpoint);
    return UNITY_END();
}
