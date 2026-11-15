# Firmware architecture

## Layer boundaries

The firmware separates startup, hardware drivers, and application work:

```text
main.c       clock, MPU/cache, peripheral and board-device initialization
app.c        application ownership and one non-blocking processing iteration
display.c    interactive display behavior
drivers      OTM8009A, FT6206, SDRAM, Ethernet, and other hardware services
```

`main()` owns the order-sensitive platform bring-up. After every required device
is ready, it binds those devices to `App_Init()` and repeatedly calls
`App_Process()`. Application code must not initialize clocks or peripheral
handles.

## HAL and CMSIS policy

Use **STM32 HAL** for peripheral initialization, transfers, interrupts, and
portable device-driver operations. HAL already encodes STM32F7 ordering and
state requirements and will remain the default as more board peripherals are
enabled.

Use **CMSIS** for Cortex-M7 core facilities that are outside the peripheral HAL:

- MPU configuration;
- instruction/data cache control and cache maintenance;
- ITM/SWO diagnostics;
- core intrinsics such as `__NOP()`, `__WFI()`, and interrupt barriers.

Direct peripheral-register access is reserved for a measured requirement that
HAL cannot meet. Such code must document the affected register, concurrency and
cache assumptions, and why HAL is unsuitable. Do not mix register-level and HAL
ownership of the same peripheral without a clearly documented boundary.

## FreeRTOS preparation

FreeRTOS is not enabled yet. The current structure prepares for it by keeping
`App_Process()` non-blocking and separating it from board initialization. A
future scheduler integration can create an application task after `App_Init()`
and replace the bare-metal loop without moving peripheral setup into a task.

Before enabling the scheduler:

1. Choose the HAL time base and ensure it remains valid after the scheduler
   starts.
2. Reserve interrupt priorities that comply with
   `configMAX_SYSCALL_INTERRUPT_PRIORITY`; the touch EXTI priority is currently
   5 and must be checked against that configuration.
3. Convert interrupt-to-task state transfer to task notifications, queues, or
   event groups where appropriate. Interrupt-shared scalar flags must remain
   `volatile` but do not by themselves provide task synchronization.
4. Define single owners or mutexes for I2C, DMA2D, DSI/LTDC, SDRAM drawing, UART,
   and lwIP.
5. Select the lwIP RTOS mode deliberately. The present configuration uses
   `NO_SYS=1` and must not be combined with the threaded tcpip core unchanged.
6. Replace indefinite waits such as `HAL_MAX_DELAY` in task-facing code with
   bounded waits or explicit ownership rules.
7. Budget task stacks and heap placement against DTCM/SRAM regions and keep
   Ethernet DMA memory non-cacheable or explicitly cache-coherent.

## Interrupt and ownership rules

Interrupt handlers do the minimum work needed to acknowledge hardware and
publish state. Display rendering, I2C transactions, logging, and protocol
processing stay in thread/main-loop context. Variables shared with interrupt
context must be documented and use suitable synchronization for the access
pattern.

External SDRAM access functions accept byte offsets from `SDRAM_START_ADDR`,
validate the complete range, and do not depend on DMA2D register state. Display
framebuffer operations remain responsible for Cortex-M7 data-cache maintenance.

---

Copyright © 2017-2026 Askug Ltd.
