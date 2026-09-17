#include <unity.h>

#ifdef __AVR__
#include "usart/usart_c.h"
#endif

#include "Controller.h"
#include "mock_temp.h"
#include "mock_timer.h"
#include "mock_relay.h"

using controller::Config;
using controller::Controller;
using controller::SetpointLimits;
using relay::MockRelay;
using temp::MockTemperature;
using temp::DeciCelsius;
using timer::MockTimer;

using State = Controller::State;

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

// One second of system time: the tick starts a conversion in interrupt
// context, then the main loop converts and acts. Two calls, because that
// split is the point.
static void aSecondPasses(Controller& unit) {
    unit.onTick();
    unit.service();
}

void setUp(void) {}
void tearDown(void) {}

static void starts_idle_with_the_heater_off(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    TEST_ASSERT_EQUAL(State::Idle, unit.state());
    TEST_ASSERT_FALSE(relay.get());
    TEST_ASSERT_EQUAL_UINT16(0, sensor.starts());
}

static void heats_when_below_the_lower_bound(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    sensor.setSample(kCold);
    aSecondPasses(unit);

    TEST_ASSERT_EQUAL(State::Heating, unit.state());
    TEST_ASSERT_TRUE(relay.get());
    TEST_ASSERT_EQUAL_INT16(148, unit.temperature().raw());
}

// The tick may not decide anything: arithmetic in an ISR is exactly what the
// specification forbids. The sample is latched on the tick and converted only
// when the main loop reads it.
static void the_tick_latches_and_the_main_loop_decides(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    sensor.setSample(kCold);
    unit.onTick();

    TEST_ASSERT_EQUAL_UINT16(1, sensor.starts());
    TEST_ASSERT_EQUAL_UINT16(kCold, sensor.readRaw());
    TEST_ASSERT_TRUE(sensor.available());
    TEST_ASSERT_FALSE(sensor.available());
    TEST_ASSERT_EQUAL(State::Idle, unit.state());
    TEST_ASSERT_FALSE(relay.get());

    // The first available() above consumed the flag, so service() must not
    // decide on a sample that was already taken.
    unit.service();
    TEST_ASSERT_EQUAL(State::Idle, unit.state());
    TEST_ASSERT_FALSE(relay.get());

    sensor.setSample(kCold);
    unit.onTick();
    unit.service();

    TEST_ASSERT_EQUAL(State::Heating, unit.state());
    TEST_ASSERT_TRUE(relay.get());
}

// Servicing without a tick must be free, or the main loop would re-decide on
// a stale sample on every pass.
static void service_without_a_tick_does_nothing(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    sensor.setSample(kCold);
    for (int i = 0; i < 10; ++i) {
        unit.service();
    }

    TEST_ASSERT_EQUAL(State::Idle, unit.state());
    TEST_ASSERT_FALSE(relay.get());
    TEST_ASSERT_EQUAL_UINT16(0, sensor.starts());
}

// available() is the only consumer of the latch flag. Reading the raw count
// must not clear it, or inspecting a sample would lose the conversion.
static void available_clears_and_readRaw_does_not(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    TEST_ASSERT_FALSE(sensor.available());

    sensor.setSample(kCold);
    unit.onTick();

    TEST_ASSERT_EQUAL_UINT16(kCold, sensor.readRaw());
    TEST_ASSERT_EQUAL_UINT16(kCold, sensor.readRaw());
    TEST_ASSERT_TRUE(sensor.available());
    TEST_ASSERT_FALSE(sensor.available());
}

// The point of hysteresis: once heating, a reading inside the band leaves the
// relay alone. A controller that switched here would chatter around setpoint.
static void keeps_heating_inside_the_band(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    sensor.setSample(kCold);
    aSecondPasses(unit);
    sensor.setSample(kInBand);
    aSecondPasses(unit);

    TEST_ASSERT_EQUAL(State::Heating, unit.state());
    TEST_ASSERT_TRUE(relay.get());
    TEST_ASSERT_EQUAL_UINT16(1, relay.transitions());
}

static void stops_heating_above_the_upper_bound(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    sensor.setSample(kCold);
    aSecondPasses(unit);
    sensor.setSample(kHot);
    aSecondPasses(unit);

    TEST_ASSERT_EQUAL(State::Idle, unit.state());
    TEST_ASSERT_FALSE(relay.get());
    TEST_ASSERT_EQUAL_UINT16(2, relay.transitions());
}

static void stays_idle_inside_the_band(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    sensor.setSample(kInBand);
    aSecondPasses(unit);

    TEST_ASSERT_EQUAL(State::Idle, unit.state());
    TEST_ASSERT_FALSE(relay.get());
    TEST_ASSERT_EQUAL_UINT16(0, relay.transitions());
}

// A single dropout is noise and must not cut the heat.
static void tolerates_fewer_bad_samples_than_the_limit(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    sensor.setSample(kCold);
    aSecondPasses(unit);

    sensor.setSample(kOpenCircuit);
    aSecondPasses(unit);
    aSecondPasses(unit);

    TEST_ASSERT_EQUAL(State::Heating, unit.state());
    TEST_ASSERT_TRUE(relay.get());
}

static void faults_and_cuts_the_heater_after_the_limit(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    sensor.setSample(kCold);
    aSecondPasses(unit);
    TEST_ASSERT_TRUE(relay.get());

    sensor.setSample(kOpenCircuit);
    aSecondPasses(unit);
    aSecondPasses(unit);
    aSecondPasses(unit);

    TEST_ASSERT_EQUAL(State::Fault, unit.state());
    TEST_ASSERT_FALSE(relay.get());
}

static void stays_off_while_faulted(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    sensor.setSample(kOpenCircuit);
    for (int i = 0; i < 10; ++i) {
        aSecondPasses(unit);
    }

    TEST_ASSERT_EQUAL(State::Fault, unit.state());
    TEST_ASSERT_FALSE(relay.get());
}

static void recovers_once_the_sensor_reads_again(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    sensor.setSample(kOpenCircuit);
    for (int i = 0; i < 5; ++i) {
        aSecondPasses(unit);
    }
    TEST_ASSERT_EQUAL(State::Fault, unit.state());

    sensor.setSample(kCold);
    aSecondPasses(unit);

    TEST_ASSERT_EQUAL(State::Heating, unit.state());
    TEST_ASSERT_TRUE(relay.get());
}

// The bad-sample counter has to reset, or a dropout every other tick would
// eventually fault a perfectly healthy sensor.
static void intermittent_dropouts_never_reach_the_limit(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    for (int i = 0; i < 20; ++i) {
        sensor.setSample(kOpenCircuit);
        aSecondPasses(unit);
        sensor.setSample(kInBand);
        aSecondPasses(unit);
    }

    TEST_ASSERT_NOT_EQUAL(State::Fault, unit.state());
}

// The high rail is a shorted thermistor, and must fault just like an open one.
static void the_high_rail_also_faults(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    sensor.setSample(1023);
    aSecondPasses(unit);
    aSecondPasses(unit);
    aSecondPasses(unit);

    TEST_ASSERT_EQUAL(State::Fault, unit.state());
    TEST_ASSERT_FALSE(relay.get());
}

static void setpoint_is_clamped_to_its_limits(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    unit.setSetpoint(DeciCelsius(4000));
    TEST_ASSERT_EQUAL_INT16(350, unit.setpoint().raw());

    unit.setSetpoint(DeciCelsius(-4000));
    TEST_ASSERT_EQUAL_INT16(50, unit.setpoint().raw());

    unit.setSetpoint(DeciCelsius(210));
    TEST_ASSERT_EQUAL_INT16(210, unit.setpoint().raw());
}

// A setpoint restored from a blank or corrupted EEPROM must not be acted on.
static void construction_clamps_an_out_of_range_setpoint(void) {
    MockRelay relay;
    MockTemperature sensor;
    Config cfg = config();
    cfg.setpoint = DeciCelsius(32767);
    Controller unit(relay, sensor, cfg);

    TEST_ASSERT_EQUAL_INT16(350, unit.setpoint().raw());
}

// The application must work through the interface, not a concrete driver.
static void controller_drives_the_sensor_through_the_interface(void) {
    MockRelay relay;
    MockTemperature sensor;
    Controller unit(relay, sensor, config());

    temp::ITemperature& asInterface = sensor;
    TEST_ASSERT_EQUAL_UINT16(0, sensor.starts());

    aSecondPasses(unit);
    aSecondPasses(unit);

    TEST_ASSERT_EQUAL_UINT16(2, sensor.starts());
    TEST_ASSERT_EQUAL_UINT16(512, asInterface.readRaw());
}

// Sampling has to be periodic, and the 1 Hz tick is what makes it so. Wired
// through the interfaces, one tick is one conversion.
static void a_timer_tick_samples_and_decides(void) {
    MockRelay relay;
    MockTemperature sensor;
    MockTimer clock;
    Controller unit(relay, sensor, config());

    clock.init(&unit);
    clock.start();

    sensor.setSample(kCold);
    clock.fire(1);
    unit.service();

    TEST_ASSERT_EQUAL_UINT16(1, sensor.starts());
    TEST_ASSERT_EQUAL(State::Heating, unit.state());
    TEST_ASSERT_TRUE(relay.get());
}

// Three ticks of an open circuit must reach the fault limit, which proves the
// bad-sample counter is driven by ticks and not by anything else.
static void repeated_ticks_on_a_dead_sensor_fault(void) {
    MockRelay relay;
    MockTemperature sensor;
    MockTimer clock;
    Controller unit(relay, sensor, config());

    clock.init(&unit);
    clock.start();

    sensor.setSample(kOpenCircuit);
    for (int i = 0; i < 3; ++i) {
        clock.fire(1);
        unit.service();
    }

    TEST_ASSERT_EQUAL_UINT16(3, sensor.starts());
    TEST_ASSERT_EQUAL(State::Fault, unit.state());
    TEST_ASSERT_FALSE(relay.get());
}

// Ticks the main loop never got to must not pile up into a backlog of
// decisions, or a slow loop would fault a healthy sensor in one pass.
static void unserviced_ticks_coalesce(void) {
    MockRelay relay;
    MockTemperature sensor;
    MockTimer clock;
    Controller unit(relay, sensor, config());

    clock.init(&unit);
    clock.start();

    sensor.setSample(kOpenCircuit);
    clock.fire(10);
    unit.service();

    TEST_ASSERT_EQUAL_UINT16(10, sensor.starts());
    TEST_ASSERT_EQUAL(State::Idle, unit.state());
    TEST_ASSERT_FALSE(relay.get());
}

// A stopped tick must not sample. Without this, stop() could silently do
// nothing and the heater would keep being driven from stale readings.
static void a_stopped_timer_never_samples(void) {
    MockRelay relay;
    MockTemperature sensor;
    MockTimer clock;
    Controller unit(relay, sensor, config());

    clock.init(&unit);

    sensor.setSample(kCold);
    clock.fire(5);
    TEST_ASSERT_EQUAL_UINT16(0, sensor.starts());

    clock.start();
    clock.fire(1);
    clock.stop();
    clock.fire(5);

    TEST_ASSERT_EQUAL_UINT16(1, sensor.starts());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(starts_idle_with_the_heater_off);
    RUN_TEST(heats_when_below_the_lower_bound);
    RUN_TEST(the_tick_latches_and_the_main_loop_decides);
    RUN_TEST(service_without_a_tick_does_nothing);
    RUN_TEST(available_clears_and_readRaw_does_not);
    RUN_TEST(keeps_heating_inside_the_band);
    RUN_TEST(stops_heating_above_the_upper_bound);
    RUN_TEST(stays_idle_inside_the_band);
    RUN_TEST(tolerates_fewer_bad_samples_than_the_limit);
    RUN_TEST(faults_and_cuts_the_heater_after_the_limit);
    RUN_TEST(stays_off_while_faulted);
    RUN_TEST(recovers_once_the_sensor_reads_again);
    RUN_TEST(intermittent_dropouts_never_reach_the_limit);
    RUN_TEST(the_high_rail_also_faults);
    RUN_TEST(setpoint_is_clamped_to_its_limits);
    RUN_TEST(construction_clamps_an_out_of_range_setpoint);
    RUN_TEST(controller_drives_the_sensor_through_the_interface);
    RUN_TEST(a_timer_tick_samples_and_decides);
    RUN_TEST(repeated_ticks_on_a_dead_sensor_fault);
    RUN_TEST(unserviced_ticks_coalesce);
    RUN_TEST(a_stopped_timer_never_samples);
    return UNITY_END();
}
