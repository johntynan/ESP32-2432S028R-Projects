# ESP32-2432S028R-Projects

The ESP32‑2432S028R is a pre‑wired ESP32 + TFT + touch module with a non‑standard SPI layout, so you must use its fixed pin assignments, custom SPI initialization, and calibrated touch mapping to get everything working smoothly.



---

# ESP32‑2432S028R: Notes for Makers  
*A practical guide to understanding this board’s unique wiring and behavior*


## Overview  
The **ESP32‑2432S028R** is a compact, integrated module that combines:

- An **ESP32‑D0WDQ6**  
- A **2.8" ST7789 TFT display**  
- A **XPT2046 resistive touch controller**  
- A shared **VSPI bus**  
- A non‑standard **pinout and wiring layout**

It’s a capable little HMI‑style device — but it does **not** behave like a typical “ESP32 DevKit + TFT + Touch” setup. This README summarizes the key differences so other makers can get productive faster.

---

## Why This Board Feels “Different”  
This board isn’t flawed — it’s just **pre‑wired** in ways that most ESP32 examples don’t expect.  
Here’s what that means in practice.

---

## 1. It’s a Pre‑Integrated Display Module, Not a Generic Dev Board  
Most ESP32 boards expose raw GPIOs and let you wire peripherals however you want.  
This board instead:

- Hard‑wires the TFT and touch controller  
- Shares the SPI bus internally  
- Uses fixed pins for MISO/MOSI/CLK/CS  
- Routes signals through a custom PCB layout

This makes it behave more like a **self‑contained HMI panel** than a breadboard‑friendly dev board.

---

## 2. The SPI Wiring Is Non‑Standard  
Typical ESP32 SPI wiring looks like:

```
MOSI = 23  
MISO = 19  
CLK  = 18  
CS   = user‑defined
```

This board uses:

```
MOSI = 32  
MISO = 39  
CLK  = 25  
TFT_CS = 15  
TOUCH_CS = 33
```

Because these are **not** the ESP32’s default VSPI pins, most libraries fail unless you explicitly initialize the SPI bus.

---

## 3. The Touch Controller Requires Manual SPI Initialization  
Unlike many XPT2046 examples, you **must** initialize a dedicated SPIClass:

```cpp
SPIClass touchSPI(VSPI);
touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
ts.begin(touchSPI);
```

If you skip this step, touch input will not work.

---

## 4. The Display and Touch Share the Same SPI Bus  
Because both devices sit on the same bus:

- You must avoid overlapping transactions  
- You must redraw UI elements in the correct order  
- Touch reads can interfere with screen updates  
- Timing matters more than on typical setups  

This is why UI elements (buttons, scrollbars, etc.) must be redrawn after every update.

---

## 5. Touch Coordinates Require Custom Calibration  
The XPT2046 raw values do not map cleanly to the screen.  
You must provide your own mapping:

```cpp
int mapX(int rawX) { return map(rawX, 259, 3633, 0, 319); }
int mapY(int rawY) { return map(rawY, 625, 3550, 0, 239); }
```

These values vary slightly between units.

---

## 6. The Board Is Powerful — It Just Expects You to Adapt to *Its* Wiring  
Nothing is “wrong” with the board.  
It simply:

- Chooses your SPI pins for you  
- Chooses your display pins for you  
- Chooses your touch pins for you  
- Requires calibration  
- Requires explicit SPI setup  

Once you work with its constraints, it becomes a very capable platform for clocks, dashboards, HMIs, and embedded UI projects.
