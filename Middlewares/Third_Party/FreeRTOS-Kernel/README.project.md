# FreeRTOS kernel

This directory contains the files required by this firmware from the official
FreeRTOS-Kernel V11.3.3 release. The upstream MIT license is retained in
`LICENSE.md`.

The selected GCC Cortex-M7 r0p1 port matches the STM32F769 hard-float build.
The initial platform, idle, and timer tasks use static allocation; heap scheme
4 is included for future dynamically allocated RTOS objects.
