# Supplying ignored source trees

The repository intentionally does not track STM32CubeF7 vendor drivers or
third-party middleware. This keeps project history focused on project-owned
firmware while allowing a checkout to use a known, compatible STM32CubeF7
distribution.

## Required local sources

A fresh checkout needs the following paths before it can build:

```text
Drivers/CMSIS/
Drivers/STM32F7xx_HAL_Driver/
Drivers/BSP/Components/lan8742/
Middlewares/Third_Party/LwIP/
```

The Makefile compiles STM32F7 HAL sources, the LAN8742 component driver, CMSIS
headers, and lwIP directly from these locations. Use source trees from one
known-good STM32CubeF7 release rather than combining arbitrary versions.

The following integration code is project-owned and tracked by Git:

```text
LWIP/App/
LWIP/Target/
Core/
```

Do not overwrite those directories while installing the vendor package.

## Preparing a checkout

1. Clone the repository and select the development branch:

   ```sh
   git clone <repository-url>
   cd stm32-f769-tmpl
   git switch dev
   ```

2. Copy the required directories from the chosen STM32CubeF7 package into the
   paths listed above.

3. Confirm that CMSIS contains the STM32F769 device headers and that the local
   toolchain provides `arm-none-eabi-gcc`.

4. Build from the repository root:

   ```sh
   make -j4
   ```

The Makefile compiles an explicit source list. Do not delete unused files from
an installed vendor tree merely to reduce the build; update the source list
when the firmware's compiled dependency set changes.

## Provenance and sharing

Record the STM32CubeF7 package name and version used for a reproducible build.
Ignored paths are not added by ordinary `git add`, which is intentional.

Before publishing vendor sources, verify their license and provenance. If the
project later needs reproducible dependency retrieval, use a reviewed submodule
or dependency-fetch procedure rather than force-adding an entire local package.
Build products, logs, editor settings, credentials, private keys, and other
machine-specific or secret-bearing files must remain untracked.

---

Copyright © 2017-2026 Dmitry Slobodchikov
