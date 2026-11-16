# FreeRTOS kernel

This directory contains the files required by this firmware from the official
FreeRTOS-Kernel V11.3.3 release. The upstream MIT license is retained in
`LICENSE.md`.

The selected GCC Cortex-M7 r0p1 port is the conservative STM32Cube-compatible
choice. The board's Cortex-M7 reports revision r1p0, which does not require the
r0p1 erratum workaround; the port remains correct at the cost of a few extra
critical-section instructions. The initial platform, idle, and timer tasks use
static allocation; heap scheme 4 is included for future RTOS objects.
