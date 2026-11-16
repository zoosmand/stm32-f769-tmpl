# Firmware architecture

## Layer boundaries

The firmware separates startup, hardware drivers, and application work:

```text
main.c       clock, MPU/cache, peripheral and board-device initialization
app.c        application ownership and one non-blocking processing iteration
console.c    USART1 command console (diagnostics), polled from the main loop
self_test.c  power-on self-test and on-demand peripheral tests
ui.c         touch UI: home, Network, and printf() Log screens
log_buffer.c line ring buffer filled from _write() for the Log screen
sntp_client.c SNTP client and software UTC clock (LWIP/App)
drivers      OTM8009A, FT6206, SDRAM, Ethernet, and other hardware services
```

`main()` owns the order-sensitive platform bring-up. It arms the console and
runs the core POST immediately after USART setup, before failure-prone board
devices. It prints a checkpoint before each major initialization stage, runs
the peripheral POST after bring-up, binds the devices to `App_Init()`, and
repeatedly calls `Console_Process()`, `MX_LWIP_Process()`, and
`App_Process()`. `App_Process()` turns completed touch taps into
`Ui_HandleTap()` calls and lets `Ui_Process()` refresh the visible screen.

The display driver and FT6206 use a bottom-left origin. The UI uses top-left
coordinates and converts at its drawing boundary; `App_Process()` converts
touch Y before calling the UI. `_write()` only appends text to the log buffer;
rendering happens later in the main loop, so `printf()` never draws. A failed
`App_Init()` is reported and leaves the console running for diagnosis. Application code must not initialize clocks or peripheral
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

## FreeRTOS ownership

FreeRTOS-Kernel V11.3.3 uses the GCC Cortex-M7 r0p1 port and a 1 kHz tick.
Board initialization, the early POST, and the peripheral POST run before the
scheduler. The scheduler then starts one statically allocated `platform` task,
which calls `Console_Process()`, `MX_LWIP_Process()`, and `App_Process()` every
millisecond. Static idle and timer tasks are also provided; heap scheme 4
reserves 64 KiB in DTCM for future RTOS objects.

FreeRTOS owns SVC and PendSV. The project SysTick wrapper always calls
`HAL_IncTick()` and calls `xPortSysTickHandler()` only after scheduler startup,
so HAL timeouts and the RTOS share the same 1 ms time base. Interrupt priority
5 is the highest priority permitted to call FreeRTOS `FromISR` APIs; touch and
LTDC use 5 and USART1 uses 6.

lwIP deliberately remains in `NO_SYS=1` raw-API mode for this migration. The
platform task is its sole owner, including DNS and SNTP callbacks. Do not call
lwIP from another task or ISR. A future threaded-lwIP migration must add a
complete `sys_arch` port and change this ownership model as one reviewed step.

The blocking `_write()` path is safe under the present single-output-task
model. Before multiple tasks log concurrently, replace it with a queue and a
dedicated UART/log task or protect it with an explicit serialization policy.

## Interrupt and ownership rules

Interrupt handlers do the minimum work needed to acknowledge hardware and
publish state. Display rendering, I2C transactions, logging, and protocol
processing stay in thread/main-loop context. Variables shared with interrupt
context must be documented and use suitable synchronization for the access
pattern.

Network services come from the DHCP lease only. lwIP's DHCP client installs DNS
servers (option 6) and calls `dhcp_set_ntp_servers()` (option 42), which the
SNTP client implements. DNS lookups and SNTP replies complete in callbacks run
by `MX_LWIP_Process()` in main-loop context, so they may call `printf()`.

USART1 reception is interrupt driven: `HAL_UART_RxCpltCallback()` (producer)
appends to a single-producer/single-consumer ring buffer that
`Console_Process()` (consumer) drains in thread context. Console commands and
tests run in the same thread as the display and therefore share I2C4, DMA2D,
and SDRAM without locking; that ownership must be revisited under an RTOS.
stdout is unbuffered and its UART writes are bounded to 100 ms per call.

External SDRAM access functions accept byte offsets from `SDRAM_START_ADDR`,
validate the complete range, and do not depend on DMA2D register state. Display
framebuffer operations remain responsible for Cortex-M7 data-cache maintenance.

---

Copyright © 2017-2026 Askug Ltd.
