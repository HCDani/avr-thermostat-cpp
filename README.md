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

- **Every driver is an abstract interface, and the test mocks implement the
  same interface.** `Controller` holds an `ITemperature&`, an `IHeater&` and
  nothing else, so the identical control logic runs against registers on the
  target and against mocks on the host with no `#ifdef` in it. This is what
  `Documentation/avr_thermostat.md` specifies, and it costs a vtable per
  interface plus the `operator delete` stub in `src/cxx_runtime.cpp` that a
  virtual destructor drags in on a part with no C++ runtime.
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

Headers are contracts and `.cpp` files are implementations. One driver per
library, each with its interface, its hardware implementation and its mock.

```
lib/thermostat/       value types and the conversion table; no AVR headers
    Types.h             AdcCount, DeciCelsius
    Thermistor.h        ADC count -> temperature, table interpolation
    ThermistorTable.h   generated
    Progmem.h           flash access on AVR, plain reads on the host
lib/hal/              hal/Adc.h                 AVR-free contract
                      src/Adc.cpp               ADC registers, ISR(ADC_vect)
lib/temp/             temp/ITemperature.h       driver interface
                      temp/temp_hw.h .cpp       NTC through the ADC, latch only
                      temp/mock_temp.h .cpp     test mock
                      temp/Conversion.h .cpp    count -> Reading, shared
lib/timer/            timer/ITimer.h            1 Hz system tick interface
                      timer/timer_hw.h .cpp     Timer 0, CTC, /1024, divide 125
                      timer/mock_timer.h .cpp   test mock, ticks on demand
lib/heater/           heater/IHeater.h          relay interface
                      heater/mock_heater.h .cpp test mock
lib/usart/            usart/IUsart.h            polled TX interface
                      usart/usart_hw.h .cpp     USART0, 115200 8N1
                      usart/mock_usart.h .cpp   test mock
lib/control/          control/Controller.h .cpp hysteresis state machine
src/                  main.cpp, cxx_runtime.cpp target entry point
test/                 host suites, Unity
tools/                table generator
```

`Types.h` and `Progmem.h` keep their bodies in the header because `constexpr`
and `inline` require the definition to be visible at the point of use.
Everything else is declared in a header and defined in a `.cpp`.

## Building and testing

Host tests need a C++ compiler on `PATH`. PlatformIO ships one, so on Windows:

```powershell
$env:PATH = "$env:USERPROFILE\.platformio\packages\toolchain-gccmingw32\bin;$env:PATH"
pio test -e native
```

Building the firmware needs no host compiler, only the AVR toolchain that
PlatformIO fetches for the board:

```powershell
pio run -e uno
```

On-target Unity, uploaded over COM9, reports through USART0 at 115200:

```powershell
pio test -e uno
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

Neither job builds the firmware yet, so `pio run -e uno` and `pio test -e uno`
are local checks. The latter flashes the board.

## Status

`Documentation/avr_thermostat.md` is the target; this is how far it has got.

Done and covered by 28 host tests: the value types, the thermistor conversion,
the hysteresis controller, and the temperature, timer, heater and USART driver
interfaces with their mocks. Compiling for the target works, and the
interrupt-driven ADC, the 1 Hz Timer 0 tick and USART0 are written and link.

Timers are allocated for the whole design, not just for what exists. Timer 1
is reserved for the servo, being the only 16-bit counter; Timer 2 is left free
because its `OC2A` output is the servo pin; the tick gets Timer 0 and divides
down to 1 Hz in software, exactly, because 16 MHz / 1024 is 5⁶.

Neither interrupt does arithmetic. The tick starts a conversion, the ADC
completion interrupt latches the raw count and sets `available()`, and the table interpolation
happens when the main loop reads it. `Controller` is split the same way:
`onTick()` for interrupt context, `service()` for the loop.

Still to come: a hardware `IHeater`, which is why `src/main.cpp` reads the
sensor instead of constructing a `Controller`; the LCD, button, encoder and
servo drivers; and EEPROM for the setpoint.

On-target Unity talks through USART0. Flash the board only when asked.
