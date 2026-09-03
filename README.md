# avr-thermostat-cpp

[![CI](https://github.com/HCDani/avr-thermostat-cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/HCDani/avr-thermostat-cpp/actions/workflows/ci.yml)

A closed-loop thermostat for an ATmega328P (Arduino Uno), written in C++14
directly against the registers with no framework underneath. A thermistor is
sampled through the ADC, the reading drives a hysteresis controller, and the
controller switches a relay.

Hardware is a Grove Starter Kit v3: thermistor on an analog input, relay as the
heater output, rotary angle sensor for the setpoint, button to commit it, RGB
LCD for display and a buzzer for faults.

## Why C++ on a part with 2 KB of SRAM

Not for the language's sake. Each of these earns its place:

- **The hardware layer is a template parameter, not an abstract base class.**
  `Controller<Hal>` dispatches statically, so there is no vtable and no
  indirect call, and the same control logic compiles against the real ADC on
  the target and against a fake on the host with no `#ifdef` in the logic.
- **`AdcCount` and `DeciCelsius` are distinct types.** A raw count cannot be
  compared against a setpoint by accident. That is a whole class of bug the
  compiler now rejects.
- **Temperatures are fixed point in tenths of a degree.** The 328P has no
  floating point unit, and 0.1 °C is already finer than the sensor's 1.5 °C
  accuracy.
- **The lookup table stays in flash.** AVR is a Harvard machine, so a const
  array in the default section is copied into SRAM at startup. `Progmem.h`
  keeps it in flash on the target and compiles down to a plain subscript on
  the host.
- **Compile-time constants are `constexpr`**, so the table geometry and the
  validity window cost nothing at runtime.

Built as C++14 rather than C++17 because PlatformIO ships g++ 5.1 for host
builds and avr-g++ 7.3 for the target, and 14 is the highest standard both
accept.

## Failing safe

The thermistor sits in a divider, so both ADC rails are electrically
meaningless: 0 claims infinite resistance, 1023 claims zero. Counts outside the
window where the sensor is specified (-40 to 125 C) are treated as a broken
sensor rather than as a temperature. A single bad sample is tolerated as noise;
several consecutive ones cut the heater and latch a fault. A setpoint restored
from a blank or corrupted EEPROM is clamped into range before it is ever acted
on.

All of that is exercised by the host suite, not by holding a lighter near the
sensor.

## Layout

```
lib/thermostat/       control logic and conversion; no AVR headers
  include/thermostat/
    Controller.h        hysteresis state machine, templated on the hardware
    Thermistor.h        ADC count -> temperature, table interpolation
    ThermistorTable.h   generated
    Types.h             AdcCount, DeciCelsius
    Progmem.h           flash access on AVR, plain reads on the host
    testing/FakeHal.h   test double
  src/ThermistorTable.cpp   generated
test/                 host suites, Unity
tools/                table generator
```

## Building and testing

Host tests need a C++ compiler on `PATH`. PlatformIO ships one, so on Windows:

```powershell
$env:PATH = "$env:USERPROFILE\.platformio\packages\toolchain-gccmingw32\bin;$env:PATH"
pio test -e native
```

Regenerating the lookup table, after changing the thermistor parameters:

```
python tools/gen_thermistor_table.py
```

The script prints reference values that the host tests assert against, so the
table and the tests cannot drift apart silently.

## Continuous integration

Every push and pull request runs two jobs:

- the host test suites, and
- a check that regenerating the lookup table produces no diff, so the table
  cannot be edited by hand and drift away from the script that owns it.

Building for the target and running the suite on the board join this once the
drivers exist.

## Status

Control logic and conversion are done and covered by 19 host tests. Still to
come: the AVR drivers (interrupt-driven ADC, timers, EEPROM, USART), a
bare-metal TWI driver for the LCD, and on-target tests through a PlatformIO
remote agent.
