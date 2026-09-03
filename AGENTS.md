# avr-thermostat-cpp

Closed-loop thermostat for an ATmega328P (Arduino Uno), C++14, bare metal, no
Arduino framework. Portable control logic lives in `lib/thermostat` and has no
AVR headers in it; host tests are in `test/`.

## Commands

Neither `pio` nor a host compiler is on `PATH`. Both ship with PlatformIO:

```powershell
$env:PATH = "$env:USERPROFILE\.platformio\packages\toolchain-gccmingw32\bin;$env:PATH"
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" test -e native
```

- Regenerate the thermistor table: `python tools/gen_thermistor_table.py`
- Run the host suite after any change under `lib/thermostat` or `test/`. It
  takes about three seconds; there is no excuse for guessing.

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
- **No board is connected.** `pio test -e uno` cannot run and that environment
  does not exist yet. Never report on-target results that did not happen.
- The Grove Temperature Sensor revision is unconfirmed. The table assumes
  B = 4275 and R0 = 100k, which is v1.1/v1.2; v1.0 boards use B = 3975 with
  R0 = 10k. Ask before changing these.

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
- Nothing under `lib/thermostat/include/thermostat/` may include an AVR header
  except `Progmem.h`, which is already guarded. That guard is what makes host
  testing possible.
- No heap on the target path: no `new`, no `std::vector`, no `std::string`, no
  `<iostream>`.
- Hardware access is a template parameter, not a virtual interface. Do not add
  abstract base classes or virtual functions to the control path — the point is
  static dispatch with no vtable.
- Temperatures are `DeciCelsius`, integer tenths of a degree. No floating point
  in anything that compiles for the target.
- Fix causes, not symptoms. Do not add compensating code downstream for a value
  that was mangled upstream.
- Implement what was asked. No extra validation, defaults or error handling
  unless requested.
