# AVR Thermostat Specs

Date: 2026

This document replaces the original VIA-shield assignment series (Key & LED,
Thermometer, 7-segment display, Solar panel control). All hardware on the VIA
shield is replaced by **Grove modules** (Seeed Studio Grove Starter Kit v3),
the MCU is an **Arduino Uno**, and the software architecture (driver layer +
application layer) stays the same. The project is written in **C++**: every
driver is an **abstract interface** with a hardware implementation, and the
**mocks used in testing are interface implementations** of the same types.

---

## 0. Hardware Replacement Overview

| Original (VIA shield)                | Replacement (Grove)                          | Notes |
|--------------------------------------|----------------------------------------------|-------|
| 4-digit 7-segment display            | **Grove - LCD RGB Backlight (16x2), row 1**  | I2C, text display |
| 8 LEDs                               | **Grove - LCD RGB Backlight, row 2**         | One character cell per "LED" |
| Switch keys (1..6) + matrix keyboard | **Grove – Button** modules (keys)            | Keys 1..3, one Grove button each |
| Matrix keyboard (value entry, A4)    | **Grove – Encoder with push button**         | Rotate = change value, press = save |
| Analogue temperature sensor TMP36    | **Grove - Temperature Sensor (NTC)**         | Thermistor, analogue output |
| Servo motor                          | **Grove Mini Servo** (same as before)        | Valve actuator |
| VIA shield                           | Grove Base Shield / direct wiring            | See §1.3 |

### LCD usage convention

- **Row 1** (16 characters) = the numeric display (replaces the 4-digit 7-segment).
- **Row 2** (16 characters) = the status indicators (replaces the 8 LEDs).
  - "LED *n*" (n = 1..8) = character position *n* in row 2.
  - state 1 → a **filled block** (custom CGRAM character 0, `█`), state 0 → space.
  - The HD44780 character ROM has no block glyph; the driver must program one
    into CGRAM at init (CGRAM holds 8 custom characters, 64×8 bits).
- Position 8 in row 2 is a spare; positions 9–16 are free for extras
  (see §6.6 backlight, optional).

---

## 1. Hardware Platform

### 1.1 Parts list

| Qty | Module (Seeed Grove)                    | From Starter Kit v3? | Role |
|-----|------------------------------------------|----------------------|------|
| 1   | Grove - LCD RGB Backlight (16x2)         | Yes                  | Numeric display + status row |
| 1   | Grove - Temperature Sensor (NTC)         | Yes                  | Panel temperature |
| 3   | Grove – Button                           | 1 in kit (buy 2 more)| Keys 1..3 |
| 1   | Grove – Encoder **with push button**     | No (kit ships a rotary *angle* sensor instead) | Setpoint entry |
| 1   | Grove Mini Servo                         | Yes                  | Valve |
| 1   | Grove Base Shield (or direct wiring)     | Yes                  | Interconnect |
| 10  | Grove cables                             | Yes                  | Wiring |
| 1   | Arduino Uno                                | — (own)              | MCU |

### 1.2 Module documentation (from Seeed wiki)

**Grove - LCD RGB Backlight** (wiki: `Grove-LCD_RGB_Backlight`)
- 16×2 characters, I2C bus, built-in English font, user-definable CGRAM
  (64×8 bits) for custom characters.
- LCD I2C address `0x3E`; RGB backlight on a second I2C address
  (`0x62` on v4, `0x30` on v5).
- Arduino library: `Grove_LCD_RGB_Backlight`
  (github.com/Seeed-Studio/Grove_LCD_RGB_Backlight).
  Basic API: `lcd.begin(16, 2); lcd.setCursor(col, row); lcd.print(...);
  lcd.setRGB(r, g, b);`

**Grove - Temperature Sensor (NTC)** (wiki: `Grove-Temperature_Sensor_V1.2`,
works with v1.1)
- NTC thermistor **NCP18WF104F03RC**, analogue output, **no enable pin**.
- Specs: R0 = **100 kΩ** at 25 °C, tolerance ±1 %, B-constant ≈ **4250–4299 K**
  (official Arduino demo uses **B = 4275**), range −40…+125 °C, accuracy ±1.5 °C.
- Conversion (official demo, NTC as top leg of a 100 kΩ divider):

```
R  = R0 * (1023.0 / a - 1.0)            // a = 12-bit ADC reading
T  = 1.0 / (log(R / R0) / B + 1/298.15) - 273.15   // degrees Celsius
```

**Grove – Encoder (incremental, with push button)** (wiki: `Grove-Encoder`)
- Incremental rotary encoder, 360° travel, two digital quadrature outputs
  (channels A, B). *This revision additionally exposes the internal push
  button as a third signal (SW)* — the base Grove-Encoder does not, so wire
  the SW pin out to a spare MCU pin.
- 4.5–5.5 V, ~20 mA.

**Grove – Button** (wiki: `Grove-Button`)
- One momentary button **with onboard pull-down** — ready to use as a
  digital input; outputs **HIGH when pressed, LOW when released**.

**Grove Mini Servo** — standard PWM servo, same as before.

### 1.3 Interconnect and pin map (Arduino Uno)

A Grove port is just VCC/GND + up to two signal wires. On the **official
Grove Base v3 shield** (designed for the Uno) the module ports map to the
following Uno pins, and the wiring below uses exactly those:

| Grove Base port | Uno pins (signal 1, signal 2) |
|:---------------:|:-----------------------------:|
| 1 | A0, A1 |
| 2 | A2, A3 |
| 3 | A4, A5 (I2C) |
| 4 | A6, A7 |
| 5 | D9, D8 |
| 6 | D10, D11 |
| 7 | D13, D12 |
| 8 | D6, D5 |

| Module        | VCC  | GND | Signal 1 (yellow) | Signal 2 (white) | Uno pin(s)            |
|---------------|------|-----|-------------------|------------------|------------------------|
| LCD 16x2      | 5V   | GND | SCL               | SDA              | A5, A4 (I2C, port 3)   |
| Temperature   | 5V   | GND | SIG (ADC)         | —                | A0 (port 1)            |
| Button key 1  | 5V   | GND | SIG               | —                | D2 (direct)            |
| Button key 2  | 5V   | GND | SIG               | —                | D3 (direct)            |
| Button key 3  | 5V   | GND | SIG               | —                | D4 (direct)            |
| Encoder A/B   | 5V   | GND | A                 | B                | D9, D8 (port 5)        |
| Encoder SW    | —    | GND | — (separate wire) | —                | D10 (internal pull-up) |
| Mini Servo    | 5V   | GND | SIG (PWM)         | —                | D11 (port 6)           |

Power: all modules are 5 V logic. The LCD, encoder and buttons are I2C/digital
only; the NTC drives an ADC input (A0). The Uno's 5 V pin supplies the
modules; keep the servo powered from a stable 5 V source (the USB supply alone
can sag under servo load), with GND common.

---

## 2. Common Software Architecture

The solution is divided into **two levels**:

1. **Driver level** — one driver per device, each in its own subfolder:
   `key/`, `led/` (LCD row 2), `display/` (LCD row 1), `temp/` (NTC),
   `encoder/`, `servo/`, plus a thin `lcd/` layer used by `led` and `display`.
   Every driver is an **abstract C++ interface**; the hardware implementations
   live in the same folder (`..._hw`) and the **test mocks are plain
   implementations of the same interface** — the application never knows which
   one it is talking to.
2. **Application level** — demo apps and the solar control app. Applications
   may **only** call the driver interfaces below; no application code touches
   pins, I2C or the Arduino API directly.

### Driver interfaces (C++)

```cpp
/* ---- lcd layer (internal; used by led & display) ---- */
class Lcd {
public:
    virtual ~Lcd() = default;
    virtual void init() = 0;
    virtual void setCursor(uint8_t col, uint8_t row) = 0;   /* row 0..1 */
    virtual void write(const char *s) = 0;
};

/* ---- led driver (LCD row 2 replaces the 8 LEDs) ---- */
class Led {
public:
    virtual ~Led() = default;
    virtual void init() = 0;
    virtual void set(uint8_t ledNo, bool state) = 0;        /* ledNo 1..8 */
    virtual bool get(uint8_t ledNo) const = 0;
};

/* ---- key driver (Grove buttons replace the VIA keys) ---- */
class Key {
public:
    virtual ~Key() = default;
    virtual void init() = 0;                                /* keys 1..3 */
    virtual bool get(uint8_t keyNo) const = 0;              /* 1 if pressed */
    /* debounce inside the driver (>= 20 ms); internal pull-up NOT needed —
       the Grove button has an onboard pull-down, pressed = HIGH. */
};

/* ---- display driver (LCD row 1 replaces the 7-segment) ---- */
class Display {
public:
    virtual ~Display() = default;
    virtual void init() = 0;
    virtual void showUInt(uint16_t value) = 0;              /* right-justified */
    /* timer tick (1 Hz) marks a refresh; the write happens on the
       next main-loop pass — no I2C inside ISRs. */
    /* optional: */
    virtual void showInt(int16_t value) = 0;
    virtual void showFloat(float value, uint8_t noOfDecimals) = 0;
};

/* ---- temperature driver (NTC replaces the TMP36) ----
   Temperatures are DeciCelsius (integer tenths of a degree), not float: the
   ATmega328P has no FPU, and 0.1 C is finer than the sensor's +-1.5 C
   accuracy. The raw sample is 10-bit, which is what this part's ADC gives
   and what the 1023 in the B-parameter formula above assumes.
   A reading carries a status, because both ADC rails are electrically
   meaningless and mean a broken sensor rather than a temperature. */
enum class SensorStatus : uint8_t { Ok, Open, Shorted };

struct Reading {
    SensorStatus status;
    DeciCelsius value;              /* meaningful only when status is Ok */
};

class ITemperature {
public:
    virtual ~ITemperature() = default;
    virtual void init() = 0;
    virtual void start() = 0;               /* one conversion; timer calls this */
    virtual bool available() = 0;           /* new latched sample; self-clearing */
    virtual uint16_t readRaw() const = 0;   /* latest 10-bit sample, latched */
    virtual Reading celsius(uint16_t raw) const = 0;
    virtual Reading get() const = 0;        /* celsius(readRaw()); main loop only */
};

/* ---- encoder driver (replaces the matrix keyboard for value entry) ---- */
class Encoder {
public:
    virtual ~Encoder() = default;
    virtual void init() = 0;
    virtual int16_t getDelta() = 0;  /* steps since last call; CW = + */
    virtual bool button() const = 0; /* true while push button held */
};

/* ---- servo driver (unchanged) ---- */
class ServoDriver {
public:
    virtual ~ServoDriver() = default;
    virtual void init() = 0;
    virtual void setAngle(uint16_t angle) = 0;              /* 0..180 */
};
```

Each folder contains `I*.h` (the interface), `*_hw.{h, cpp}` (the hardware
implementation) and `mock_*.h/.cpp` (the test mock, e.g. `MockKey` lets tests
programmatically press keys, `MockLcd` records what was written to row 1/row 2,
`MockEncoder` lets tests inject rotation steps and button presses).

### Timing / interrupt strategy (all parts)

- **Timer 0 in CTC mode is the 1 Hz system tick.** Timer 1 is reserved for the
  servo and Timer 2 for the servo's output pin. Each tick starts one ADC
  conversion and sets the application's "refresh due" flag.
- **ADC completion interrupt (`ADIE`)**: the ISR only latches `ADCL/ADCH` and
  sets the driver's available flag. The ADC is **not** in continuous mode, it
  converts once per tick.
- **Conversion to degrees happens in the main loop**, when `available()` is
  true. Both ISRs must stay **as short as possible** (no I2C, no arithmetic).
- The *main loop* performs the LCD refresh and value writes when the
  "refresh due" flag is set (I2C transactions must not run inside ISRs).
  This fulfils the original requirement *"update of the display must be timer
  interrupt driven"*: the interrupt drives the update, the write happens in
  loop context.
- **Encoder quadrature** is evaluated in pin-change interrupts on D8/D9
  (short ISR: update a 2-bit state machine, accumulate the step count).
- Keys and the encoder button are polled with debounce in the main loop.

---

## 3. Part 1: Key and Status-Display Drivers

Design and implement drivers for the **Grove buttons** (keys) and the
**LCD row 2** (status indicators), with the interfaces in §2.

- Part 1.1 — status ("LED") output driver: `Led::init()`, `Led::set(ledNo,
  state)` with ledNo 1..8 mapped to row 2, positions 1..8.
- Part 1.2 — key input driver for **three** Grove buttons: `Key::init()`,
  `Key::get(keyNo)` for keyNo 1..3.
- Part 1.3 — **Demo application**: row 2 must show the results of six logic
  operators between the states of **key 1** and **key 2**:

  | Key 1 | Key 2 | Pos 1 AND | Pos 2 OR | Pos 3 XOR | Pos 4 NAND | Pos 5 NOR | Pos 6 XNOR |
  |:-----:|:-----:|:---------:|:--------:|:---------:|:----------:|:---------:|:----------:|
  |   0   |   0   |     0     |     0    |     0     |     1      |     1     |     1      |
  |   0   |   1   |     0     |     1    |     1     |     1      |     0     |     0      |
  |   1   |   0   |     0     |     1    |     1     |     1      |     0     |     0      |
  |   1   |   1   |     1     |     1    |     0     |     0      |     0     |     1      |

  (Positions 7..16 of row 2 stay off.)

## 4. Part 2: Thermometer (NTC)

Design and implement a thermometer that reads the **Grove Temperature Sensor
(NTC)** on A0.

Requirements:

1. The thermometer must autonomously, **once every second** (the 1 Hz tick
   starts the conversion and `ADIE` latches the sample — see §2), convert the
   sample to temperature in degrees Celsius using the B-parameter formula in
   §1.2.
2. Display the temperature as a **bar in LCD row 2** (extend your status
   driver with a bar feature, e.g. `Led::bar(lo, hi, value)` or similar).
   Range **18–25 °C**: position 1 lights if T ≥ 18 °C, all 8 positions light
   if T ≥ 25 °C; intermediate positions fill proportionally.

## 5. Part 3: Numeric Display Driver (LCD Row 1)

Design and implement a driver for **row 1 of the 16x2 LCD**, replacing the
4-digit 7-segment driver.

Requirements:

1. Interface: `Display::init()`, `Display::showUInt(uint16_t value)` (right-
   justified in row 1, leading zeros like the original 7-seg display).
2. **Updates must be timer interrupt driven** — the 1 Hz timer tick marks a
   refresh; the driver writes row 1 on the next main-loop pass (§2).
3. Divide the code into logical abstraction layers: display-driver functions
   separated from application code, interacting only via the interface above.
4. Write an application that shows the **temperature** on the numeric display
   (reuses the Part 2 temperature driver).

Optional (not required to pass):

- Signed version: `Display::showInt(int16_t value)`.
- Floating point: `Display::showFloat(float value, uint8_t noOfDecimals)`.
- (The original "load the display via SPI" option is **dropped** — the LCD
  is an I2C device.)

## 6. Part 4: Solar Panel Control (Application)

Design and implement a simplified **solar heating control system** using the
Arduino Uno with the Grove modules.

The device measures the **panel temperature** with the NTC sensor and controls
a **valve (servo motor)** and a **pump (status indicator)**:

- The servo opens/closes the district-heating circuit depending on the energy
  production from the sun.
- The pump circulates the brine between the solar panel and the storage tank
  (no real pump — the pump state is shown in row 2, position 7).

Behaviour:

- When the temperature drops **below tlow**: the pump must **stop** and the
  valve must **open**.
- When the temperature rises **above thigh**: the valve must **close** and
  the pump must **start**.
- Between tlow and thigh (hysteresis band): keep the current state.
- The setpoints tlow and thigh are **configurable by the user** (initial
  values tlow = 18 °C, thigh = 25 °C).

Use the already implemented drivers: **keys (Grove buttons), status row
(LCD row 2), numeric display (LCD row 1), NTC thermometer, encoder (with
push button), servo**.

### 6.1 Display and mode selection

- The **current temperature** is shown in row 1 (numeric display), e.g.
  `TEMP   23 C`.
- Pressing **key 1** switches row 1 to **tlow** (`TLOW   18 C`); row-2
  position 1 on, positions 2–3 off.
- Pressing **key 2** switches row 1 to **thigh** (`THIGH  25 C`); row-2
  position 2 on, positions 1 and 3 off.
- Pressing **key 3** switches row 1 back to the current temperature; row-2
  position 3 on, positions 1 and 2 off.
- **Row-2 position 6** is on if the **valve is open**, off if closed
  (valve driven by the servo: 0° = closed, 120° = open — implement as named
  constants).
- **Row-2 position 7** is on while the **pump is running**, off otherwise.

### 6.2 Setpoint entry (encoder replaces the matrix keyboard)

When row 1 shows tlow or thigh:

- **Rotating the encoder** adjusts the value: 1° per detent, CW increases,
  CCW decreases, clamped to **0…60 °C**.
- A **short press** of the encoder push button **saves** the value
  (replaces the `#` key).
- A **long press** (≥ 1 s) **cancels** the change and restores the previous
  value (replaces the `*` key).

(While row 1 shows the current temperature the encoder has no effect.)

### 6.3 Optional

- Return automatically to the current-temperature display **5 seconds**
  after tlow or thigh have been shown/changed (use a timer tick).
- Use the LCD **RGB backlight** to indicate the active mode (e.g. white =
  temperature, blue = tlow, red = thigh).

### 6.4 Testing

(To be designed later — see §8.)

---

## 7. Non-functional Requirements

1. The solution must be divided into **two levels: drivers and application**.
   Each driver must be in its own subfolder (see §2). You may reuse the key
   and status drivers from the previous parts.
2. The temperature sensor driver must start **one conversion per 1 Hz tick**
   with an **ADC completion interrupt** (`ADIE`) — not continuous conversion.
   Design all ISRs to be **as short as possible** (no I2C, no arithmetic, no
   blocking in ISRs).
3. Source code must be documented by **inline comments**, including author
   name(s) and date.
4. Use **Test Driven Development** (see §8 for the test plan). Tests run on
   the host PC against the **mock implementations** of the driver interfaces
   (same C++ types as the hardware drivers — §2); the application code under
   test must therefore depend on the interfaces, never on the hardware
   classes.

## 8. Testing

> **Placeholder — the testing section will be designed later.**
> (Test-case specifications from the original assignments are intentionally
> left out of this revision; TDD remains a requirement, the concrete test
> cases will be added in a separate document.)

## 9. What to hand in

1. **Class diagram** of the driver/application structure.
2. **Sequence diagram** of the interaction between the solar control
   application and the drivers.
3. **Timing diagram** of one display-update cycle (timer tick → refresh →
   I2C transfer). Use the wavedrom tool: https://wavedrom.com/
4. **Activity diagram** of the display driver.
5. **UML domain diagram** of the solar control application.
6. All source files in a **zip archive or link to a GitHub repository**
   (production code; the test project is added when the test plan lands).
