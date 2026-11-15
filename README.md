# STM32F769I-DISCO firmware template

Bare-metal STM32 HAL firmware for the **STM32F769I-DISCO** board and its
STM32F769NIH6 Cortex-M7 microcontroller. The project is intended to become a
well-defined, fully initialized board platform on which applications can be
built without repeating low-level bring-up work.

The repository is under active development. The current firmware demonstrates
the onboard display and touchscreen; it is not yet a complete initialization
of every board peripheral.

## Current capabilities

The current `dev` firmware configures:

- a 216 MHz system clock sourced from the board's 25 MHz HSE;
- MPU, instruction cache, and data cache;
- internal Flash execution and the STM32F769 memory layout;
- external SDRAM through FMC;
- the 800 x 480 OTM8009A LCD through DSI Host and LTDC;
- DMA2D-backed drawing, double framebuffers, and bitmap fonts;
- the FT6206 capacitive touchscreen over I2C;
- USART1 and ITM/SWO output for `printf`;
- CRC and board GPIO;
- a bare-metal lwIP/Ethernet implementation with LAN8742 and DHCP code.

Ethernet initialization and processing are currently disabled in `main.c`.
Other onboard interfaces, QSPI boot, and HDMI support remain future work.

## Requirements

- GNU Arm Embedded toolchain providing `arm-none-eabi-gcc`, `objcopy`, and
  `size`;
- GNU Make;
- an ST-LINK-compatible programmer/debugger;
- local STM32CubeF7 driver and middleware trees described in
  [Supplying ignored source trees](docs/IGNORED_SOURCES.md).

If the toolchain is not on `PATH`, pass its binary directory to Make:

```sh
make GCC_PATH=/path/to/arm-gnu-toolchain/bin -j4
```

## Build

From the repository root:

```sh
make -j4
```

The build creates these primary files under `build/`:

```text
F769_eth_test_06.elf
F769_eth_test_06.hex
F769_eth_test_06.bin
F769_eth_test_06.map
```

Remove generated output with:

```sh
make clean
```

The linker script currently places code in the MCU's 2 MiB internal Flash,
normal data in DTCM, Ethernet buffers in a dedicated SRAM region, and the stack
and heap in SRAM2. External SDRAM is initialized at runtime and used for display
framebuffers.

## Runtime behavior

After reset, the firmware initializes the clock tree and core memory
attributes, starts SDRAM and the display pipeline, initializes the touch
controller, and enters the interactive display loop. Diagnostic output is sent
to both USART1 and ITM stimulus port 0.

## Repository layout

```text
Core/                         Application and board initialization
Drivers/                      Local STM32CubeF7 drivers (ignored by Git)
LWIP/                         Project-owned lwIP and Ethernet integration
Middlewares/Third_Party/LwIP/ Local lwIP sources (ignored by Git)
docs/                         Development documentation
Makefile                      GNU Arm build definition
STM32F769XX_FLASH.ld          Internal-Flash linker script
startup_stm32f769xx.s         Cortex-M7 startup and vector table
stm32f769.svd                 MCU peripheral description
```

## Development direction

Bring-up will proceed in small, hardware-verifiable steps. Planned areas
include fault diagnostics, remaining board peripherals, storage and networking,
HDMI integration, QSPI programming and memory-mapped operation, and finally a
documented QSPI boot flow. Cache/MPU behavior, DMA coherency, clock constraints,
pin conflicts, and memory consumption must be reviewed as each subsystem is
enabled.

Contributors should read:

- [Development rules](docs/DEVELOPMENT_RULES.md)
- [Naming conventions](docs/NAMING_CONVENTIONS.md)
- [Supplying ignored source trees](docs/IGNORED_SOURCES.md)

## Reference

- [STM32F769 SVD](https://github.com/tinygo-org/stm32-svd/blob/main/svd/stm32f769.svd)

---

Copyright © 2017-2026 Askug Ltd.
