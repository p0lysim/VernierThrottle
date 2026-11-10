# Electronics and firmware

[← Back to the README](../README.md)

## Components

| Part | Notes |
|---|---|
| Arduino Micro (ATmega32u4, 16 MHz) | Native USB, required for the HID game controller. A Leonardo works too |
| AS5600 board | 12-bit magnetic angle sensor, I²C address `0x36`, raw angle registers `0x0C`/`0x0D`. Square board (≈ 23 × 23 mm, 4 holes) screwed on the back of the AS5600 holder |
| Diametric magnet, Ø 5 × 2 mm | In the rear pocket of the `Magnet_End_Cap`, centred over the AS5600 chip, about 0.5 to 3 mm away |
| Rotary potentiometer, linear (B), 10 kΩ, 6 mm shaft | Single turn, driven by the pinion |

## Wiring

```
        Arduino Micro                          AS5600 board
       ┌────────────┐                        ┌────────────┐
       │        5V  ├────────────┬───────────┤ VCC        │  check whether your board expects 5 V or 3.3 V
       │       GND  ├─────────┬──┼───────────┤ GND        │
       │   D2 / SDA ├─────────┼──┼───────────┤ SDA        │  pull-ups are usually on the board
       │   D3 / SCL ├─────────┼──┼───────────┤ SCL        │
       │            │         ├──┼───────────┤ DIR        │  tie to GND, never leave it floating
       │            │         │  │           └────────────┘
       │            │         │  │            Potentiometer
       │            │         │  └───────────┤ end 1      │
       │        A5  ├─────────┼──────────────┤ wiper      │
       │            │         └──────────────┤ end 2      │
       └─────┬──────┘                        └────────────┘
             │ USB → PC
```

- The AS5600 board moves with the slider: use flexible stranded wire and leave a loop for the full travel
  of the knob.
- If the axis goes the wrong way when you push, swap the two outer wires of the potentiometer.
- If the fine adjustment goes the wrong way, tie DIR to the other supply rail, or remove the `*= -1` line
  in `main.cpp`.

### Pro Micro

On a SparkFun Pro Micro and its clones, **A4 and A5 are not available on the pins**. Wire the
potentiometer to A0–A3, change `#define POT_PIN A5` in `firmware/src/main.cpp`, and use
`board = sparkfun_promicro16` in `firmware/platformio.ini`. SDA and SCL stay on D2 and D3.

## Build and flash

The firmware is a [PlatformIO](https://platformio.org/) project. Open the `firmware/` folder in VS Code
with the PlatformIO IDE extension, or use the command line:

```bash
cd firmware
pio run
pio run -t upload
```

- The only dependency, the Arduino Joystick Library, is bundled in `firmware/lib/`.
- During the upload, the board restarts in its bootloader and may show up on another COM port for a few
  seconds. That is normal.
- If the upload cannot find the board, press RESET twice quickly and start the upload again within
  8 seconds.

Memory use: about 12 KB of flash (42 %) and 570 bytes of RAM (22 %).

## What the PC sees

| Field | Value |
|---|---|
| Device name | "Arduino Micro" (to rename it, add `board_build.usb_product = "Vernier Throttle"` to `platformio.ini`) |
| Type | HID joystick, report ID 3 |
| Axes | **Throttle** only, values 0–1023 (sent as 0–65535) |
| Buttons | 32 declared, not used |

The board also exposes a serial port, which is used for uploads and debugging.

## Test on Windows

1. Plug in the throttle, press `Win + R` and run `joy.cpl`.
2. Select **Arduino Micro**, then **Properties**.
3. Check that:
   - the throttle bar follows the knob over its full travel;
   - one turn of the knob moves the bar by about 10 %;
   - fully pulled out reads 0 % and fully pushed in reads 100 %, even after twisting the knob;
   - the value stays still when you let go of the knob (if it drifts, see
     [Tuning tips](how-it-works.md#tuning-tips)).

## Debugging

`main.cpp` contains a commented-out block that prints the output and both increments on the serial port
whenever something changes. To use it:

1. uncomment the `debug_current_value_*` variables and initialise them;
2. uncomment the lines that set them and the print block at the end of `loop()`;
3. run `pio device monitor -b 115200`.

Leave this block commented out in normal use: printing on every loop slows the firmware down.
