# CLAUDE.md

Firmware for the LHR Solar **BPS Leader** board (STM32G473, FreeRTOS, STM32 HAL). The git repo root is one level up (`../`), which also holds the KiCad design in `../Hardware`.

## Current use: BQ79600 + BQ79616 testbench

This board and firmware are currently being used as a **testbench to talk to TI BQ79616(-Q1) battery monitors through a TI BQ79600(-Q1) bridge**. The Leader board's UART goes to a **BQ79600 EVM**, and the BQ79600 talks to the BQ79616(s) over the isolated COMH/COML daisy chain. The goal is to bring up and validate the host-side driver before it goes into the real BPS. Unless told otherwise, assume new work is about this bring-up.

- **Datasheets** (kept in the repo; check them for register addresses, frame formats, timing and state-machine behavior instead of relying on memory, and cite section or table numbers in code comments):
  - [bq79600-q1.pdf](bq79600-q1.pdf): BQ79600-Q1 bridge (SLUSDS1A, 71 pages). This is the device the MCU actually talks to, so pings, UART physical layer, auto-addressing and bridge registers come from here.
  - [bq79616.pdf](bq79616.pdf): BQ79616 monitor (SLUSF21A, 215 pages). Use it for monitor registers, ADC, balancing and fault behavior.
  - Search them as text first (`pdftotext -layout <pdf> out.txt`, then grep), and render individual pages only for figures and timing diagrams.
- **Driver**: [Core/Inc/bq.h](Core/Inc/bq.h), [Core/Src/bq.c](Core/Src/bq.c). (It was developed as `new_bq.*` and replaced the old BQ79616-direct driver; that old driver is gone.)
  - `bq_init()` takes a `bq_init_struct_t`: pins, UART handle and AF, wake timer/channel/AF, and `num_devices` (BQ79616s in the stack, 0..`BQ_MAX_DEVICES`, not counting the bridge). It rejects anything over `BQ_MAX_DEVICES`.
  - `bq_wake()` sends the WAKE ping, then `CONTROL1[SEND_WAKE]` to the bridge, and waits `num_devices × BQ_WAKE_PER_DEV_MS` for the stack.
  - `bq_auto_address()` runs bq79600-q1.pdf Table 7-9 steps 3–10 after a wake: bridge = 0, BQ79616s = 1..`num_devices`, last one top of stack. It then reads the addresses and the bridge's `DEV_CONF1` back to check them. It needs `num_devices >= 1`.
  - Commands are described by a `bq_msg_frame_t` (`req`, `dev_addr`, `reg_addr`, `data`, `len`) passed by pointer. `bq_write(&msg)` sends a write frame; for `bq_read(&msg, frames, max_frames)`, `msg.len` is the bytes to read per device and `msg.data` is unused. `bq_read()` fills a caller-provided `bq_response_frame_t` array (`dev_addr` + `data` per responding device) and takes its length. It returns `BQ_ERR` without sending if the array is too small; `BQ_MAX_RESPONSE_FRAMES` entries always fit.
- **Bench tests**:
  - [Tests/new_bq_test.c](Tests/new_bq_test.c), `make TEST=new_bq`: minimal. It inits and wakes the chain, then blinks the heartbeat; no heartbeat means init or wake failed.
  - [Tests/bq_test.c](Tests/bq_test.c) (+ [Tasks/Inc/bq_test.h](Tasks/Inc/bq_test.h)), `make TEST=bq`: checks `bq_init()`/`bq_wake()` on boot and records the result in `bq_test_fail_step`, `bq_test_passed` and `bq_test_wake_us` for GDB (on failure the FAULT LED turns on and the heartbeat stops). It then clears the bridge's reset fault; auto-addressing is still to be written. `BQ_TEST_NUM_DEVICES` in `bq_test.h` sets how many BQ79616s are behind the bridge.
- **Wiring**: PB10 (USART3 TX) → BQ79600 RX, PB11 (USART3 RX) ← BQ79600 TX, BQ79600 NFAULT → PC8 (the FAN_SCL pin, borrowed for the bench), common GND. On the EVM, nUART/SPI must be tied to GND for UART mode, and nCS must be low, or the BQ79600 ignores pings (Section 5, Section 7.3.1.2).
- **Link**: USART3, AF7, 1 Mbps (BQ79600 default, Section 7.3.2.1.2.1), 8N1. PB11 has a pull-up because the BQ79600's TX is high-Z in SHUTDOWN.
- **Debug printf**: moved to **UART4** (PC10/PC11, AF5, 115200) via `debugPrintf_init_uart(huart4)`, because USART3 belongs to the BQ. `HAL_UART_MspGPIOInit` in [DebugPrintf.c](Drivers/Src/DebugPrintf.c) only configures pins for the debug UART; the BQ driver sets up PB10/PB11 itself.

### Protocol notes (section references are to `bq79600-q1.pdf` unless noted)
- **Addressing**: the BQ79600 is always the base device, address 0. The BQ79616s are stack devices at addresses 1..N, with the highest one set as top of stack. The bridge's own registers are mostly at `0x2000`+, but it shares `DIR0_ADDR` (`0x0306`) and `CONTROL1` (`0x0309`) with the monitors (Section 7.5).
- **WAKE ping**: wakes **only the BQ79600** (Section 7.3.1.2). Hold its RX low for tHLD_WAKE = 2.5–3 ms (Section 6.6), then wait tSU(WAKE_SHUT) (3.5 ms max) before talking to it. `bq.c` generates the pulse with a timer channel (TIM2_CH3 on PB10, one-pulse mode) so the length is set in hardware, not by CPU timing. `bq_wake()` must run after `bq_init()` and from a task, since it calls `vTaskDelay`. If the BQ79600 was shut down with a SHUTDOWN ping, it needs two WAKE pings, tSU(WAKE_SHUT) apart.
- **Waking the stack**: after the ping, single-write `CONTROL1[SEND_WAKE]` (bit 5) on the BQ79600 (device 0) to send the WAKE tone up the daisy chain. Then wait for each BQ79616 to wake (its tSU(WAKE_SHUT), `bq79616.pdf` Section 5.6).
- **Auto-addressing**: follow Table 7-9 (Section 7.3.2.3.1). It differs from the BQ79616-only sequence: the DLL sync is 8 separate dummy stack writes to `0x343`–`0x34A`, and later 8 separate dummy stack reads of the same registers.
- **Command frame**: `INIT | [DEV ADR] | REG ADR (2, MSB first) | DATA (1–8) | CRC (2, LSB first)`. The INIT byte is `0x80 | REQ_TYPE<<4 | (len-1)`. DEV ADR is present only for single read/write (Section 7.3.2.1.1).
- **Read**: the DATA byte holds (bytes to return − 1). Each device sends back one frame: `INIT(len-1) | DEV ADR | REG ADR (2) | DATA | CRC`. A stack read returns one frame per stack device, top of stack first.
- **No broadcast reads of monitor data**: through the bridge, use only single or stack reads. In a broadcast read the BQ79600 answers for addresses it doesn't have with `0x00` (Section 7.3.2.1.1 notes).
- **CRC**: CRC-16-IBM (poly 0x8005, init 0xFFFF, reflected in/out), computed with the STM32 hardware CRC unit. Running the CRC over a full frame that includes its CRC gives 0 (Section 7.3.2.1.1.1–2).
- **Sanity checks**: a single read of `DEV_CONF1` (`0x2001`) from device 0 (the BQ79600) should return its reset value `0x14` (Table 7-9 step 10). After auto-addressing, a single read of PARTID (`0x0500`) from device 1 should return `0x21`, the BQ79616 (`bq79616.pdf` Section 6.5).
- **NFAULT**: open-drain, active low, enabled by `DEV_CONF1[NFAULT_EN]` (on by default). It reports the bridge's faults, plus stack faults only if those are forwarded down the chain (Section 7.3.3.2).

### Test plan docs (not yet updated for the bridge)
[test.txt](test.txt) (T1–T5) and [BQ79616_TEST_GUIDE.md](BQ79616_TEST_GUIDE.md) were written for a BQ79616 wired straight to the UART, with the old serial-menu `bq_test.c`. They don't account for the bridge: for example, they ping the BQ79616 directly and assume it is device 0. Most of the T1–T5 constants in `bq_test.h` are left over from that menu.

## Build / flash / debug

The build is driven by the `Embedded-Sharepoint` submodule (UT LHR shared HAL/BSP/FreeRTOS and Makefiles). The toolchain comes from Nix: run `./run_nix.sh` (`nix develop ./Embedded-Sharepoint`) first.

```bash
make                 # normal BPS firmware (Core/Src/main.c)
make TEST=new_bq     # build Tests/new_bq_test.c instead of main.c
make TEST=bq         # build Tests/bq_test.c
make flash           # flash via ST-Link/OpenOCD
make flash-uart      # flash via USB cable
make debug           # OpenOCD + GDB
python3 Tests/makeAllTests.py   # compile every test
```

- `TEST=<name>` builds `Tests/<name>_test.c` and leaves out `Core/Src/main.c`. Every test file has its own `main()`.
- Everything matching `*/Src/*.c` and `*/Inc` is compiled and included automatically. There is no source list to update.
- Output goes to `build/`. `bear` (if installed) writes `.vscode/compile_commands.json`.
- Target: `PROJECT_TARGET=stm32g473xx`.

## Layout
- `Core/`: `main.c`, `common.h` (the umbrella include for FreeRTOS, HAL, config, pindef and the fault handler), the fault handler, and the BQ driver
- `Config/Inc/`: `config.h` (task priorities and stack sizes, thresholds), `pindef.h` (board pin map), CAN message and receive tables
- `Drivers/`: board drivers (ADC, CAN, contactors, EMC2305 fans, SHT45, IWDG, status LEDs, debug printf)
- `Tasks/`: FreeRTOS tasks for the normal BPS application (init, voltage/temperature/current monitors, precharge, contactors, fans, CAN, watchdog, fault handler)
- `Tests/`: standalone hardware test programs, one per file
- `Embedded-Sharepoint/`: submodule. Don't edit it here; it provides `UART.h` (`husart3`, `huart4`, `uart_init/send/recv`), `printf`, CAN, and other drivers.

## Conventions
- FreeRTOS tasks are created with `xTaskCreateStatic` and statically allocated buffers and stacks. Stack size and priority are `#define`s in a header (`*_TASK_STACK_SIZE`, `*_PRIO`).
- Drivers return a status enum (`BQ_OK`/`BQ_ERR`/…, `UART_OK`/…) and don't call `Error_Handler()` on runtime failures.
- Comment the hardware "why" (datasheet timing, pin glitch avoidance) next to the code that depends on it.
