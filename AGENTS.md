# avr-thermostat-cpp

Closed-loop thermostat for an ATmega328P (Arduino Uno), C++14, bare metal, no
Arduino framework. Host tests are in `test/`.

`Documentation/avr_thermostat.md` is the **goal**, not a description of what
exists. It is the authority on architecture; where it and this file disagree,
say so rather than picking one silently.

One library per driver, each holding an interface, a hardware implementation
and a mock: `lib/adc`, `lib/temp`, `lib/timer`, `lib/heater`, `lib/usart`,
`lib/twi`, `lib/lcd`, `lib/led`, `lib/key`. The thermostat application is
`lib/controller`; the Part 1 demo is `lib/logic`. Only `lib/adc/Adc.cpp`,
`lib/temp/temp_hw.cpp`, `lib/timer/timer_hw.cpp`, `lib/usart/usart_hw.cpp`,
`lib/twi/Twi.cpp`, `lib/lcd/lcd_hw.cpp`, `lib/key/key_hw.cpp` and `src/`
touch AVR headers.

## Commands

Neither `pio` nor a host compiler is on `PATH`. Both ship with PlatformIO:

```powershell
$env:PATH = "$env:USERPROFILE\.platformio\packages\toolchain-gccmingw32\bin;$env:PATH"
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" test -e native
```

- Build the firmware: `pio run -e uno`. Needs no host compiler on `PATH`.
- On-target tests: `pio test -e uno`. This **flashes** COM9. Do not run it
  unless asked.
- Regenerate the thermistor table: `python tools/gen_thermistor_table.py`
- Run the host suite after any change under `lib/` or `test/`, and
  `pio run -e uno` after any change that the target compiles. Together they
  take about ten seconds; there is no excuse for guessing.

## Toolchain facts you cannot see

- PlatformIO Core 6.1.18. Host compiler is the bundled MinGW **g++ 5.1**, target
  compiler is **avr-g++ 7.3**.
- **C++14 is the ceiling**, set by g++ 5.1. No `if constexpr`, no inline
  variables, no structured bindings, no `std::optional`.
- g++ 5.1 defaults to `gnu++98`, so `-std=gnu++14` must stay in `build_flags`.
- **PlatformIO ignores `extra_scripts` in this project.** A script that appends
  compiler flags will not run, and the symptom is every `constexpr` in the
  headers being rejected as "does not name a type". Put flags in `build_flags`.
- Target budget: 2 KB SRAM, 32 KB flash, 1 KB EEPROM. Compiled with
  `-fno-exceptions -fno-rtti`.
- **A board is connected on COM9.** `pio run -e uno` builds and
  `pio run -e uno -t upload` flashes it. `pio test -e uno` uploads Unity
  through USART0 at 115200 8N1 and reports over that same port. Never report
  on-target results that did not happen, and never flash the board without
  being asked.
- **Timer allocation is fixed.** Timer 1 is the servo: it is the only 16-bit
  counter, and SIG is D9 (`OC1A`), so the pulse is hardware PWM, not an ISR
  toggle. The encoder is on D6/D7; its switch is D12 on the Arduino header.
  Timer 2 stays unused (`OC2A` is D11, not on the Grove shield; `OC2B` is D3,
  key 2). The 1 Hz tick is Timer 0 in CTC with a software divide by 125:
  16 MHz / 1024 = 15625 = 5^6, so 125 divides it exactly and the tick does
  not drift. A `static_assert` fails the build if a clock or prescaler
  change ever makes that division inexact.
- **`ADTS` has no 1 Hz source**, so the ADC cannot be triggered by hardware
  from the tick: Timer 0 offers only compare match A at 125 Hz and Timer 2 is
  not a trigger source at all. The tick starts each conversion in software.
- `lib_ldf_mode = deep` is deliberate. The default `chain` will not follow
  `lib/temp` to `hal/Adc.h`, and `deep+` evaluates `#ifdef __AVR__` while
  scanning and so finds no includes at all in the guarded files.
- The Grove Temperature Sensor is an NCP18WF104F03RC with R0 = 100k and
  B = 4275, confirmed by the documentation. The generated table matches.
- **The Base Shield 2.0 has no I2C pull-ups**; it only wires the four I2C
  connectors to A4/A5. Arduino sketches work because `Wire`'s `twi_init()`
  silently enables the AVR's internal pull-ups, so `Twi::init()` does the same
  explicitly. Without them SDA/SCL never rise and the hardware cannot even
  finish a START, which reads as `TWSR` never setting `TWINT`. Those pull-ups
  are 20-50k against the 4.7-10k I2C wants, so the bus is stiff enough only
  on short cables; external resistors would be better.
- **The LCD controller NACKs while it is busy.** A string must go out as one
  transfer (control byte then all data bytes), and each transfer needs about
  40 us afterwards, or every byte after the first is dropped.
- **The LCD ignores the AiP31068 contrast registers.** Measured on the board
  across the whole 0..63 range: no effect, because the module fixes the LCD
  bias in hardware, which is why Seeed's own library never sets them. Do not
  re-add a contrast sequence. Backlight PWM is the only readability lever, and
  full brightness washes the characters out; `kBacklightLevel` is 64. The panel
  keeps a narrow viewing cone regardless.

## Rules

- **Never run git.** No commits, pushes, branches or tags. Dániel does all git
  operations himself.
- `ThermistorTable.h` and `ThermistorTable.cpp` are generated. Edit
  `tools/gen_thermistor_table.py` and re-run it. Never hand-edit either file.
- Tests and source may both be changed, and a failing test does not
  automatically mean the source is wrong. Decide which one encodes the mistake,
  then say which you changed and why. Do not make a test pass by weakening it.
- Reference values in the tests come from the generator's printed output. If the
  sensor parameters change, regenerate the table and update the assertions in
  the same change.
- No interface header, no application code and nothing under
  `lib/thermostat/include/thermostat/` may include an AVR header, except
  `Progmem.h`, which is already guarded. Registers belong in the `*_hw.cpp`
  files. That separation is what makes host testing possible.
- No heap on the target path: no `new`, no `std::vector`, no `std::string`, no
  `<iostream>`. `src/cxx_runtime.cpp` defines `operator delete` because a
  virtual destructor references it, but it is an unreachable linker stub, not
  an allocator, and there is deliberately no `operator new`.
- Hardware access is an abstract interface, per the documentation: `I*.h`
  declares it, `*_hw.{h,cpp}` implements it against registers, `mock_*.{h,cpp}`
  implements it for tests, and the application holds only the interface. This
  reverses an earlier rule that mandated template parameters for static
  dispatch; the documentation won.
- Temperatures are `DeciCelsius`, integer tenths of a degree. No floating point
  in anything that compiles for the target.
- Fix causes, not symptoms. Do not add compensating code downstream for a value
  that was mangled upstream.
- Implement what was asked. No extra validation, defaults or error handling
  unless requested.
- Headers musn't contain implementation. The only exceptions are `Types.h` and
  `Progmem.h`, where `constexpr` and `inline` require the definition to be
  visible at the point of use. Do not reintroduce templates in the driver or
  control path, which would force bodies back into headers.
- Both ISRs must stay short, per the specification: no I2C and no arithmetic.
  The ADC completion interrupt latches the raw count and sets
  `ITemperature::available()`; that call is self-clearing. Conversion to
  `DeciCelsius` happens when the main loop sees the flag and calls
  `ITemperature::get()`. Do not move arithmetic back into either ISR, and do
  not reintroduce a push-style listener that would deliver readings from
  interrupt context.
- `TemperatureHw::readRaw()` masks interrupts around the 16-bit load. The ISR
  writes that latch at roughly the moment the tick reads it, so an unguarded
  read can tear on an 8-bit part.
- The application is split across the two contexts: `Controller::onTick()` is
  interrupt context and only starts a conversion, `Controller::service()` is
  main-loop context and does the arithmetic and the switching once
  `available()` is true. Unserviced samples coalesce deliberately; a backlog
  of decisions would fault a healthy sensor in one pass.
- `Documentation/avr_thermostat.md` is a specification. Keep it short and
  compact: state what the design is, not why it was chosen. Rationale,
  register-level detail and toolchain workarounds belong here or in the
  README.
