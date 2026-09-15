# avr-thermostat-cpp

[![CI](https://github.com/HCDani/avr-thermostat-cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/HCDani/avr-thermostat-cpp/actions/workflows/ci.yml)

A closed-loop thermostat for an ATmega328P (Arduino Uno), written in C++14
directly against the registers with no framework underneath. A thermistor is
sampled through the ADC, the reading drives a hysteresis controller, and the
controller switches a relay.

Hardware is a Grove Starter Kit v3: NTC on A0, relay as the heater output,
encoder for the setpoint, buttons, RGB LCD, and a servo in the longer-term
spec. `Documentation/avr_thermostat.md` is that goal, not a description of
what already runs.

## Why C++ on a part with 2 KB of SRAM

Not for the language's sake. Each of these earns its place:

- **Every driver is an abstract interface, and the test mocks implement the
  same interface.** `Controller` holds an `ITemperature&` and an `IHeater&`,
  so the identical control logic runs against registers on the target and
  against mocks on the host with no `#ifdef` in it. That costs a vtable per
  interface plus the `operator delete` stub in `src/cxx_runtime.cpp` that a
  virtual destructor drags in on a part with no C++ runtime.
- **`DeciCelsius` is a distinct type.** A raw ADC count cannot be compared
  against a setpoint by accident.
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
meaningless: 0 claims infinite resistance, 1023 claims zero. Counts outside
24..992 (the window where the sensor is specified, −40 to 125 °C) are a
broken sensor rather than a temperature: open lead vs short. A single bad
sample is tolerated as noise; several consecutive ones cut the heater and
latch a fault. A setpoint restored from a blank or corrupted EEPROM is
clamped into range before it is ever acted on.

`temp::convert()` is the one place that mapping happens. Host tests exercise
it; the ISRs never do.

## Layout

Headers are contracts and `.cpp` files are implementations. Libraries are
flat: PlatformIO's include root is the library folder, so sources include
`"Adc.h"`, not `"hal/Adc.h"`. One driver per library, each with its
interface, its hardware implementation and its mock.

```
lib/adc/          Adc.h .cpp              interrupt-driven ADC, AVR-free header
lib/temp/         ITemperature.h          driver interface
                  temp_hw.h .cpp          NTC through the ADC: latch + available()
                  mock_temp.h .cpp        test mock
                  Conversion.h .cpp       count -> Reading (status + DeciCelsius)
                  DeciCelsius.h           tenths of a degree
                  ThermistorTable.h .cpp  generated lookup
                  Progmem.h               flash access on AVR, subscript on host
lib/timer/        ITimer.h                1 Hz system tick
                  timer_hw.h .cpp         Timer 0, CTC, /1024, divide by 125
                  mock_timer.h .cpp       ticks on demand
lib/heater/       IHeater.h               relay interface
                  mock_heater.h .cpp      test mock (no GPIO driver yet)
lib/usart/        IUsart.h                polled TX
                  usart_hw.h .cpp         USART0, 115200 8N1
                  usart_c.h               C API for Unity
                  mock_usart.h .cpp       test mock
lib/twi/          Twi.h .cpp              blocking I2C master
lib/lcd/          ILcd.h                  16x2 text; used by led and display
                  lcd_hw.h .cpp           Grove LCD RGB Backlight over TWI
                  mock_lcd.h .cpp         2x16 write buffer
lib/led/          ILed.h                  status cells on LCD row 2
                  led_hw.h .cpp           CGRAM block / space; talks only to ILcd
                  mock_led.h .cpp         test mock
lib/display/      IDisplay.h              numeric display on LCD row 1
                  display_hw.h .cpp       row 1: BAR 21-28C and e.g. " 21.5C"
                  mock_display.h .cpp     test mock
lib/key/          IKey.h                  Grove buttons 1..3
                  key_hw.h .cpp           D2/D3/D4, debounce >= 20 ms
                  mock_key.h .cpp         test mock
lib/controller/   Controller.h .cpp       hysteresis; onTick vs service
lib/logic/        Demo.h .cpp             Part 1.3 key states then AND/OR/XOR/NAND/NOR/XNOR
lib/thermo/       Thermometer.h .cpp      Part 2+3: bar 21–28 C, row 1 range + XX.XC
src/              main.cpp, cxx_runtime.cpp
test/             Unity host suites; unity_config.h for the Uno
tools/            table generator
```

`DeciCelsius.h` and `Progmem.h` keep their bodies in the header because
`constexpr` and `inline` require the definition at the point of use.
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

That command flashes the board.

Regenerating the lookup table, after changing the thermistor parameters:

```
python tools/gen_thermistor_table.py
```

The script prints reference values that the host tests assert against, so the
table and the tests cannot drift apart silently. `ThermistorTable.h` and
`ThermistorTable.cpp` are generated; do not edit them by hand.

## Continuous integration

Every push and pull request runs two jobs:

- the host test suites, and
- a check that regenerating the lookup table produces no diff, so the table
  cannot be edited by hand and drift away from the script that owns it.

Neither job builds the firmware, so `pio run -e uno` and `pio test -e uno`
are local checks.

## Status

`Documentation/avr_thermostat.md` is the target; this is how far it has got.

Done and covered by host tests: conversion (`convert()` plus the generated
table), the hysteresis controller, the temperature, timer, heater and USART
interfaces with their mocks, and Part 1 (LCD status row, Grove keys, logic
demo). The interrupt-driven ADC, the 1 Hz Timer 0 tick, USART0 and a blocking
TWI master are written. Firmware currently runs Parts 2 and 3: the NTC is sampled
once a second; LCD row 2 shows a 21–28 °C bar and row 1 shows `BAR 21-28C`
plus the reading as tenths of a degree, e.g. ` 21.5C`.

Timers are allocated for the whole design. Timer 1 is the servo: hardware PWM
on `OC1A` (D9, white of Grove port D8). The encoder is D6/D7, switch on D12.
Timer 2 is unused. The tick is Timer 0 in CTC, divided by 125 in software.

Neither interrupt does arithmetic. The tick starts a conversion; the ADC
completion interrupt latches the count and sets `available()`; interpolation
runs in the main loop via `ITemperature::get()`. `Controller` is split the
same way: `onTick()` in interrupt context, `service()` in the loop. The Part 1
demo has no ISR of its own: keys are polled and I2C runs in the main loop.
`Thermometer` is split like `Controller`: `onTick()` starts a conversion and
marks a display refresh, `service()` converts, draws the bar, and writes row 1.

Still to come: encoder and servo, a hardware `IHeater`, and EEPROM for the
setpoint.
