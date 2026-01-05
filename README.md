# Features
- OSDSYS hooking

# Supported models

Only 77k and 2.20 90k were tested, but should at least work on any other 2.20 model.

Other non-protokernel models untested.

Protokernel models not supported (10k and 15k).

# Hardware requirements

- RP2040 board x1
- 1k-3.3k Ω resistor x2
- 3.3k-4.7k Ω resistor x1
- Diode x1 (depending on RP2040 board)

A low-profile board like PR2040-Tiny is recommended.

# Build instructions

Recursively clone the repository so that submodules are initialized.

## Requirements
- [PS2SDK](https://github.com/ps2dev/ps2dev)
- Arm C23 compiler (Clang 19+ or GCC 15+)

[Arm Toolchain for Embedded 20.1.0](https://github.com/arm/arm-toolchain) was used during development.

## Building

Pick one of the board configurations in `fruitchip-fw-board` or make your own,
see `fruitchip-fw-board/custom.cmake.example` for example board configuration.

```sh
export PATH=/opt/ATfE-20.1.0-Linux-x86_64/bin:$PATH
cmake -B build -DBOARD=waveshare-rp2040-tiny
cmake --build build -j$(nproc)
```

# Installation

Find a modchip installation diagram for your model, locate boot ROM, power, ground and reset points, and connect them accordingly.

For example, for Modbo 4.0 and RP2040-Tiny:

| Modbo 4.0 | PS2  | RP2040-Tiny | Notes                      |
|-----------|------|-------------| ---------------------------|
| V         | Q0   | GPIO0       | -                          |
| U         | Q1   | GPIO1       | -                          |
| T         | Q2   | GPIO2       | -                          |
| Q         | Q3   | GPIO3       | -                          |
| P         | Q4   | GPIO4       | -                          |
| O         | Q5   | GPIO5       | -                          |
| N         | Q6   | GPIO6       | -                          |
| M         | Q7   | GPIO7       | -                          |
| W         | CE   | GPIO9       | Needs a 1k-3.3k Ω resistor |
| R         | OE   | GPIO8       | Needs a 1k-3.3k Ω resistor |
| 3.3V      | 3.3V | 5V          | Needs a diode              |
| GND       | GND  | GND         | -                          |
| RST       | RST  | GPIO10      | Needs a 1k-4.7k Ω resistor |

※ RP2040 can't be powered from 3V3 pin.

※ 5V pin accepts 3.3V-5V for power in.

※ 5V pin on RP2040-Tiny needs a diode to prevent backfeed from USB, make sure voltage doesn't drop below 3.3V.

# Debug color meaning

## Loader
| Color | Meaning |
|-----|--------|
| ![700000](https://img.shields.io/badge/-700000?style=for-the-badge) | Stage 1 failed: modchip command failed |
| ![000070](https://img.shields.io/badge/-000070?style=for-the-badge) | Stage 1 failed: checksum did not match |
| ![007000](https://img.shields.io/badge/-007000?style=for-the-badge) | Stage 2 failed: failed to execute stage 3 |


## RGB LED

| Color | Meaning |
|-----|--------|
| ![FFFF00](https://img.shields.io/badge/-FFFF00?style=for-the-badge) | Bootloader running              |
| ![FF0000](https://img.shields.io/badge/-FF0000?style=for-the-badge) | Apps partition header not found |
| ![00FF00](https://img.shields.io/badge/-00FF00?style=for-the-badge) | Ok                              |
| ![0000FF](https://img.shields.io/badge/-0000FF?style=for-the-badge) | Settings failed to initialize   |
| ![00FFFF](https://img.shields.io/badge/-00FFFF?style=for-the-badge) | OSDSYS hook succeeded           |
| ![FF00FF](https://img.shields.io/badge/-FF00FF?style=for-the-badge) | Unused                          |
| ![FFFFFF](https://img.shields.io/badge/-FFFFFF?style=for-the-badge) | Unused                          |

※ Board may or may not have RGB LED

## Non-RGB LED

| Color | Meaning |
|-----|--------|
| ![FF0000](https://img.shields.io/badge/-FF0000?style=for-the-badge) | Power on                                    |
| ![0000FF](https://img.shields.io/badge/-0000FF?style=for-the-badge) | Read activity: Boot ROM pins in output mode |

※ Board may or may not have power LED and/or LED connected to GPIO
