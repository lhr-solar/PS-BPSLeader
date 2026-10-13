# BQ79616 Bench Test Guide

This guide covers how to run the BQ79616 test plan ([test.txt](test.txt)) using the BPS Leader board as the host controller. It also explains what the test firmware does and how it does it.

- **Test firmware:** [Tests/bq_test.c](Tests/bq_test.c), with settings in [Tasks/Inc/bq_test.h](Tasks/Inc/bq_test.h)
- **Driver:** [Core/Src/bq.c](Core/Src/bq.c), [Core/Inc/bq.h](Core/Inc/bq.h)
- **Datasheet:** [bq79616.pdf](bq79616.pdf) (TI SLUSF21A). Section and table numbers below refer to this document.

> **Status:** the firmware compiles but hasn't been run against an EVM yet. Treat the first run of each test as a check of the test itself as well as of the hardware. See [Known limitations](#known-limitations).

---

## Part 1: Running the tests

### 1.1 Equipment

| Item | Used by | Notes |
|---|---|---|
| BPS Leader board | all | The host MCU (STM32G473) |
| ST-Link programmer | all | For flashing (`st-flash`) |
| 3.3 V USB-serial adapter | all | Connects to the debug console on UART4 |
| BQ79616EVM ×1 | T1, T2, T4.1, T4.3, T4.4, T5 | |
| BQ79616EVM ×2 + daisy-chain cable | T3, T4.2 | |
| Cell simulator or adjustable supplies | T1, T5 | 8 different cell voltages (T1.1 uses 3.2–4.1 V) |
| One isolated supply per EVM | T3, T4 | Required by T3.1. Lets you measure each board's current separately. |
| Ammeter that can read µA | T4 | Shutdown current is expected to be 10–40 µA |
| 2-channel oscilloscope | T5.1–T5.5, T4 (optional) | CH1 on PB10, CH2 on a CB node |
| DMM | T1, T2.1, T5.6 (optional) | |
| Heat gun | T2.3 | |
| Reference thermometer | T2.2 (optional) | |

### 1.2 Wiring

**Leader board to EVM (BQ UART, USART3, 1 Mbps):**

| Leader pin | Connects to | Direction |
|---|---|---|
| PB10 (USART3 TX) | EVM RX | Leader → BQ |
| PB11 (USART3 RX) | EVM TX | BQ → Leader. PB11 has an internal pull-up, because the BQ's TX floats in SHUTDOWN. |
| GND | EVM GND | Required |

In a daisy chain, the Leader connects only to the **base** EVM. The second EVM connects to the base through the EVM's COMH/COML daisy-chain connector.

**Leader board to PC (debug console, UART4, 115200 8N1):**

| Leader pin | Connects to |
|---|---|
| PC10 (UART4 TX) | adapter RX |
| PC11 (UART4 RX) | adapter TX |
| GND | adapter GND |

The debug console uses UART4 because USART3 is reserved for the BQ.

### 1.3 EVM setup per test group

These steps come from test.txt. Set the hardware up before running the command.

- **T1:** The EVM's 0 Ω straps only go down to 12S. For 8S, short VC9–VC16 and CB9–CB16 to the top cell at the J15 harness connector. Apply a different voltage to each of cells 1–8. The defaults the test expects are 3.2, 3.4, 3.5, 3.6, 3.7, 3.8, 4.0 and 4.1 V. To change them, edit `BQ_TEST_CELL_EXPECTED_V`. You don't need to set the cell count in the TI GUI, because the firmware writes `ACTIVE_CELL` itself.
- **T2:** Fit J4 shunts on all 8 GPIO pairs and populate J5 (the TSREF pull-up). The firmware configures `GPIO_CONF` and enables TSREF itself.
- **T3, T4.2:** Two EVMs in a daisy chain, each on its own isolated supply. Run `n 2` before these tests.
- **T4.3, T5:** The cells must be able to supply balancing current, so use real cells or a simulator that can sink and source current.

### 1.4 Build, flash, connect

From `Software/`:

```bash
./run_nix.sh
```

Then, inside the Nix shell:

```bash
make TEST=bq
```

```bash
make flash
```

Open the console. Replace the device path with your adapter's:

```bash
picocom -b 115200 --omap delbs --echo /dev/ttyUSB0
```

(This is what `Embedded-Sharepoint/start-serial.sh` runs.) The firmware reads a line when you press Enter, which sends a carriage return (`\r`). Both picocom and PuTTY do this by default. The firmware prints each command back after you press Enter. With `--echo` on, you'll see the command twice, which is harmless.

> If you rebuild without `TEST=bq`, `build/` ends up holding the normal BPS firmware. Always run `make TEST=bq` before `make flash`.

### 1.5 What you should see at boot

```
BQ test start
bq_init OK
bq_wake took 4xxxx us (min 42250 us)
PASS
OK: BQ79616 found
BQ79616 test plan. For the two-board chain, run 'n 2' first.
Commands (devices in chain: 1):
  ...
bq> 
```

Before the menu appears, the firmware checks the driver's bring-up:

1. Calling `bq_wake()` before `bq_init()` must be rejected.
2. `bq_init()` must succeed. Afterwards USART3 must be enabled at the right baud rate, PB10/PB11 must be in alternate function AF7, and PB11 must have its pull-up on.
3. `bq_wake()` must succeed and take at least the ping plus settle time. Afterwards PB10 must be back on USART3 and idling high.

If any of these fails, the board prints `FAIL: <step>`, turns on the FAULT LED, stops the heartbeat and halts. You can read the failed step in GDB with `p bq_test_fail_step`. Otherwise the heartbeat LED blinks every 500 ms the whole time, including while the menu waits for input.

The firmware then tries to read PARTID. With one EVM attached this should print `OK: BQ79616 found`. With two chained EVMs that haven't been addressed yet, this read is expected to fail. Run `n 2` and the tests will address the chain.

### 1.6 Menu commands

Type a command and press Enter. Arguments in `[ ]` are optional.

| Command | Argument | What it does |
|---|---|---|
| `help` | | List the commands |
| `n` | `1` or `2` | Set how many devices are in the chain (default 1). This affects every command. |
| `link` | | Wake, auto-address and check PARTID on every device |
| `shut` | | Send a broadcast shutdown to every device |
| `stat` | `[dev]` | Dump FAULT_SUMMARY, FAULT_SYS, FAULT_COMM1, DEV_STAT, ADC_STAT1, BAL_STAT, COMM_CTRL and ACTIVE_CELL |
| `bal` | `[off]` | Show the remaining balancing time for cells 1–8, or stop all balancing |
| `t11` | | T1.1 |
| `t21` / `t22` / `t23` / `t24` | `t22 [ambient °C]`, `t24 [period ms]` | T2.1–T2.4 |
| `t31`–`t35` | `t34 [auto]` | T3.1–T3.5 |
| `t41` / `t42` / `t43` | `[code]` | T4.1–T4.3. `code` is the timeout: 1 = 100 ms, **2 = 2 s (default)**, 3 = 10 s, 4 = 1 min |
| `t44` | `[period ms] [code]` | T4.4 |
| `t51` | `[hex mask]` | T5.1. The mask selects cells 1–8, e.g. `0x55` = cells 1, 3, 5, 7 (default) |
| `t52` | `[cell]` | T5.2 |
| `t53` / `t54` / `t55` | | T5.3–T5.5 |
| `t56` | `[mV]` | T5.6. The argument is the error threshold (default 1.0 mV) |

### 1.7 Reading the output

Every test prints a header, a list of checks, and a verdict:

```
=== T5.3: Pause ===
  waiting for C1 to drop below 280s...
  [ ok ] BAL_STAT 0x28: CB_INPAUSE = 1
  [ ok ] CB_RUN stays 1 while paused
  C1 held at 275s; waiting 15 s
  [ ok ] timer frozen: 275s -> 275s
  OPERATOR: Every CB node should be off now.
=== T5.3 PASS (firmware checks; finish the OPERATOR checks) ===
```

- `[ ok ]` / `[FAIL]` are checks the firmware made itself. Any `[FAIL]` makes the test FAIL.
- `OPERATOR:` lines are checks only you can make, with a meter, scope or by eye. If a test prints any of these, the verdict says "finish the OPERATOR checks", and the test only fully passes once you've done them.
- `>> ... [Enter]` means the test is waiting for you. Do what it says, then press Enter.
- Register reads that fail print the reason: `TIMEOUT` (no reply), `CRC` (corrupted reply), `FRAME` (reply from the wrong device or register), or `ERR` (bad argument or driver not initialized).

To fill in "Actual result" in test.txt, copy the check lines plus your own measurements for the OPERATOR lines.

### 1.8 Suggested run order

1. **Single EVM, `n 1`:** `t11`, `t21`, `t22 <ambient>`, `t23` (×3 channels), `t24`.
2. **Single EVM, balancing:** `t51`, `t52`, `t53`, `t54`, `t55`, `t56`. T5.2–T5.5 continue from the balancing state the previous test left behind. Each one starts balancing first if it isn't running, so they also work on their own.
3. **Single EVM, timeouts:** `t41`, `t43`, `t44`.
4. **Two EVMs, `n 2`:** `t31`, `t32`, `t33`, `t34`, `t35`, `t42`.

Run `bal off` before you disconnect the cells.

---

### 1.9 Test-by-test procedures

#### T1.1: 8S strapping and channel mapping (`t11`)

- **Setup:** 8S strapping as in [1.3](#13-evm-setup-per-test-group), with different voltages on cells 1–8.
- **Firmware:**
  1. Sets `ACTIVE_CELL` to 8S, starts the main ADC in continuous mode with the 26 Hz filter, and reads it back to confirm 8S.
  2. Averages 10 readings of VCELL1–8.
  3. Compares each cell to its expected voltage (±`BQ_TEST_CELL_TOL_MV`, default 10 mV). If a cell is off, it says which other cell's voltage it matches, so a channel swap is obvious.
  4. Prints VCELL9–16.
- **Pass:** all 8 cells are within tolerance on the right channels.
- **Operator:** check that VCELL9–16 look as expected. Channels above the cell count aren't converted and should read `0x8000`.

#### T2.1: TSREF under full load (`t21`)

- **Setup:** J4 shunts on all 8 GPIO pairs, J5 fitted.
- **Firmware:**
  1. Sets all 8 GPIOs to "ADC only" and turns TSREF on.
  2. Reads back `GPIO_CONF1..4` (expects `0x12` each) and checks that `CONTROL2[TSREF_EN]` = 1.
  3. Averages 10 TSREF readings.
  4. Checks that every GPIO reads like a connected thermistor (between 0.1% and 99.9% of TSREF), which confirms that all 8 dividers are loading TSREF.
- **Pass:** TSREF is between 4.945 and 5.055 V. That's the datasheet's 4.975–5.025 V widened by its ±30 mV load-regulation spec. All 8 dividers must also be present.
- **Operator (optional):** measure TSREF with a DMM.

#### T2.2: GPIO readback at ambient (`t22 [ambient]`)

- **Firmware:** averages 10 readings and converts each GPIO to °C ([2.6](#26-measurements-and-conversions)).
- **Pass:**
  - With an ambient value (e.g. `t22 23.5`): each channel is within ±3 °C of it.
  - Without one: all 8 channels are within a 3 °C spread of each other.
- **Operator:** rerun with a reference-thermometer reading if you didn't give one.

#### T2.3: Thermistor channel mapping (`t23`)

- **Firmware:**
  1. Takes a baseline.
  2. Prints a row of temperature changes per channel about every 0.5 s until you press Enter.
  3. Reports the channel with the biggest rise.
- **Procedure:** run `t23`, heat **one** thermistor with the heat gun, and press Enter once it has clearly warmed. Repeat for at least 3 different channels.
- **Pass:** the heated channel rose at least 5 °C (a rise confirms the conversion's direction is right). Every other channel moved no more than 2 °C.
- **Operator:** confirm that the reported channel is the one you heated.

#### T2.4: Combined poll cycle (`t24 [period ms]`)

- **Firmware:** runs 100 polls at the chosen period (default 100 ms). Each poll is **one** read that returns VCELL1–8, TSREF and GPIO1–8 together, from every device in the chain. It times each read with the CPU cycle counter.
- **Pass:** every poll returned converted data (no `0x8000`), and no read took longer than the poll period.
- **Also reported:** the polls per second achieved and the longest single read. Use these to choose the poll speed you intend to run.

#### T3.1: Wake (`n 2`, then `t31`)

- **Firmware:**
  1. Shuts down every device, then wakes the base with a WAKE ping. The base passes the wake on up the chain.
  2. Auto-addresses the chain.
  3. Reads PARTID from every device with a broadcast read.
- **Why addressing is part of this test:** an un-addressed stack device only accepts broadcast writes, so the firmware can't hear back from it until it has an address.
- **Pass:** every device answers with PARTID `0x21`.

#### T3.2: Auto-addressing (`t32`)

- **Firmware:** wakes and addresses the chain as in T3.1, then reads each device's `DIR0_ADDR` and `COMM_CTRL` individually.
- **Pass:**
  - Device 0: address 0, STACK_DEV = 0, TOP_STACK = 0.
  - Device 1 (top of stack): address 1, STACK_DEV = 1, TOP_STACK = 1.

#### T3.3: Stack and broadcast reads (`t33`)

- **Firmware:** starts the ADC on every device, then runs both kinds of read:
  - A **stack** read, which only stack devices answer. With 2 EVMs that's one reply, from device 1.
  - A **broadcast** read, which every device answers, top of stack first.

  It prints each device's cell voltages, TSREF and GPIO temperatures.
- **Pass:** each read returns the expected number of replies, from the right devices in the right order (top first), all with valid data.

#### T3.4: Re-enumeration repeatability (`t34` or `t34 auto`)

- **Firmware:** runs 10 cycles. Each cycle shuts the chain down, prompts you to power-cycle both boards, then repeats T3.1, T3.2 and T3.3.
- **Procedure:** at each prompt, switch both EVM supplies off and on again, then press Enter. `t34 auto` skips the prompts and only uses the firmware shutdown, which is quicker but isn't a true power cycle.
- **Pass:** all 10 cycles succeed.

#### T3.5: Position independence (`t35`)

- **Procedure:** when prompted, power down, swap the two boards' positions in the chain, power up and press Enter. The firmware then runs T3.1–T3.3 with no configuration changes.
- **Pass:** T3.1–T3.3 all pass.

#### T4.1: Timeout to shutdown (`t41 [code]`)

- **Setup:** a µA ammeter in each EVM supply, and optionally a scope on PB10 and the supply current.
- **Firmware:**
  1. Wakes and addresses the chain. Writes a marker value (`0xA5`) into a scratch register on every device, then programs the communication timeout (default 2 s) with SHUTDOWN as its action.
  2. Starts the ADC and polls for 3 s, then stops all traffic part-way through polling.
  3. At **80%** of the timeout, reads every device's marker. All devices must still answer. This read also restarts the timeout.
  4. Stays silent for **120%** of the timeout plus the 20 ms shutdown time, then reads PARTID. There must be **no reply**.
  5. Prompts you to measure the current, then wakes the chain and reads the markers. They must now be `0x00`, which proves each device went through a reset.
- **Pass:** all firmware checks pass. Together they place the shutdown between 80% and 120% of the timeout.
- **Operator:**
  - While the prompt is waiting, measure each board's supply current. It should be 10–40 µA.
  - For the exact shutdown time, scope the last frame on PB10 against the drop in supply current.

#### T4.2: Stack device shutdown (`n 2`, then `t42 [code]`)

- **Phase A (firmware-verified):** the timeout is set on the **top** device only. After 120% of the timeout, the base must still answer and the top device must not. Because the base never shut down, this proves the stack device shut down by itself.
- **Phase B:** the same as T4.1, with the timeout on every device.
- **Why two phases:** when the base wakes, it sends a wake signal up the chain, and that resets the stack device whether or not it shut down. So Phase B alone can't show that the stack device timed out.
- **Operator:** in Phase B, measure the current on **both** boards' supplies.

#### T4.3: Balancing stops on shutdown (`t43 [code]`)

- **Firmware:** the same as T4.1, but it starts balancing first (default cells 1, 3, 5, 7). It confirms balancing is running when traffic stops, and confirms balancing is not running after the wake.
- **Operator:** while the measurement prompt is waiting, confirm that no CB node is switching and there's no balancing current.

#### T4.4: No false trips (`t44 [period ms] [code]`)

- **Firmware:** sets the timeout, then polls every device at the chosen period (default 100 ms). It polls for 5× the timeout or 30 s, whichever is longer. At the end it checks that every device's marker is still `0xA5`. A device that had timed out would have been reset and lost its marker. The firmware then turns the timeout off again.
- **Pass:** every poll is answered and every marker is still in place.

#### T5.1: Selective channel enable (`t51 [hex mask]`)

- **Firmware:**
  1. Makes sure the device is set to 8S (channels above the cell count are skipped during balancing).
  2. Stops any balancing, then sets a 300 s timer on the selected cells and starts balancing in manual mode.
  3. Checks that the chip accepted the configuration (`INVALID_CBCONF` = 0) and that balancing is running (`CB_RUN` = 1).
  4. Reads the remaining time for every cell from 1 to 8.
- **Pass:** selected cells show time remaining, and the other cells show none.
- **Note:** the chip refuses adjacent channels by default (`DEV_CONF[NO_ADJ_CB]` = 1), so the mask must not include two cells next to each other.
- **Operator:** check at the CB nodes that only the selected channels are switching. Balancing is left running for T5.2–T5.5.

#### T5.2: Independent channel control (`t52 [cell]`)

- **Firmware:**
  1. Stops one channel by setting its timer to 0 and restarting balancing. This is the datasheet's documented way to stop a single channel (6.3.3.3.2).
  2. Checks that this channel stopped, the others are still balancing, and `CB_RUN` is still 1.
  3. Prompts you, then starts the channel again.
- **Note:** restarting balancing reloads every channel's timer, so the other channels' remaining times jump back to 300 s. They don't stop balancing.
- **Operator:** check the CB nodes at the prompt and again at the end.

#### T5.3: Pause (`t53`)

- **Firmware:**
  1. Waits until the first selected cell's timer is below 280 s, so that a resume can be told apart from a restart in T5.4.
  2. Sets `CB_PAUSE` = 1.
  3. Checks that `CB_INPAUSE` = 1 and `CB_RUN` is still 1.
  4. Reads the remaining time, waits 15 s, and checks it hasn't changed.
- **Operator:** check that every CB node is off. Balancing is left paused for T5.4.

#### T5.4: Resume (`t54`)

- **Firmware:**
  1. If balancing isn't paused, pauses it first.
  2. Reads the held time, then clears `CB_PAUSE`.
  3. Checks that the time right after resuming is within 5 s of the held value and at least 10 s below the full 300 s, so the timer wasn't restarted.
  4. Checks that 15 s later the time is lower, so it's counting down again.
- **Operator:** check that the CB nodes are switching again.

#### T5.5: Pause/resume latency (`t55`)

- **Setup:** scope CH1 on PB10 (the Leader's TX, i.e. BQ RX), triggered on a falling edge. CH2 on a CB node of a selected cell.
- **Firmware:** for PAUSE and then RESUME:
  1. Prompts you to arm the scope.
  2. Sends the command (a 7-byte frame, 70 µs on the wire).
  3. Reports when the UART finished sending, then polls `BAL_STAT` as fast as it can and reports when the new state appears.
- **Operator:** from the scope, record the time from the first edge on PB10 to the FET turning off (PAUSE) and on (RESUME). The `BAL_STAT` times the firmware prints are only an upper bound.

#### T5.6: Settle delay sweep (`t56 [mV]`)

- **Firmware:**
  1. With balancing **off** and the ADC filter off, takes 10 single-shot readings of cells 1–8 as the baseline. It reports the worst cell's peak-to-peak noise.
  2. Starts balancing and waits 2 s.
  3. Takes readings with no pause and reports the error. This shows how much error pausing has to remove.
  4. For each delay of 0, 1, 2, 3, 5, 7, 10, 15, 20, 30, 50, 75 and 100 ms, runs 5 times: pause → wait the delay → take a reading → read the result → resume → wait 250 ms. It reports the worst cell's average error against the baseline.
  5. Stops balancing.
- **Pass:** some delay brings the error to the threshold or below (default 1.0 mV; use something like `t56 0.5` to change it). The test reports the shortest such delay.
- **Timing note:** the real settle time is the delay plus about 140 µs of command frames, plus up to one 192 µs ADC conversion cycle.
- **Operator (optional):** compare the baseline against a DMM on the cell inputs.

### 1.10 Settings you can change

All of these are in [Tasks/Inc/bq_test.h](Tasks/Inc/bq_test.h). Rebuild after changing any of them.

| Setting | Default | Meaning |
|---|---|---|
| `BQ_TEST_CELL_EXPECTED_V` | 3.2 … 4.1 V | T1.1: voltages applied to cells 1–8 |
| `BQ_TEST_CELL_TOL_MV` | 10 mV | T1.1: how far a cell may be off |
| `BQ_TEST_NTC_R_PULLUP` / `_R25` / `_BETA` | 10 kΩ / 10 kΩ / 3380 | **Check these against the EVM parts list.** T2.2 and T2.3 depend on them. |
| `BQ_TEST_TSREF_MIN_V` / `_MAX_V` | 4.945 / 5.055 V | T2.1 limits |
| `BQ_TEST_AMBIENT_TOL_C` | 3 °C | T2.2 |
| `BQ_TEST_HEAT_RISE_C` / `_OTHERS_C` | 5 / 2 °C | T2.3 |
| `BQ_TEST_POLL_PERIOD_MS` / `_CYCLES` | 100 ms / 100 | T2.4, T4 |
| `BQ_TEST_CTL_CODE_DEFAULT` | 2 (2 s) | T4 timeout |
| `BQ_TEST_BAL_MASK_DEFAULT` | `0x55` | T5 cells (1, 3, 5, 7) |
| `BQ_TEST_BAL_TIMER_CODE` | `0x04` (300 s) | T5 balancing timer |
| `BQ_TEST_PAUSE_HOLD_MS` | 15 s | T5.3 / T5.4 wait |
| `BQ_TEST_SETTLE_SAMPLES` / `_ERR_MV` | 5 / 1.0 mV | T5.6 |

### 1.11 Troubleshooting

| Symptom | Likely cause |
|---|---|
| `FAIL: init periph` at boot | USART3 or its pins didn't configure. Check that `HAL_CRC_MODULE_ENABLED` is set in the Embedded-Sharepoint `stm32g4xx_hal_conf.h`. |
| `can't read device ID (status TIMEOUT)` at boot | TX/RX swapped, no common GND, EVM not powered, or two un-addressed EVMs connected (expected; run `n 2`) |
| `auto-address: TIMEOUT` or `FRAME` | `n` doesn't match the number of boards connected, or there's a problem with the daisy-chain cable |
| Frequent `CRC` errors | Wiring or ground noise at 1 Mbps. Shorten the leads. |
| `link lost` / tests re-linking unexpectedly | The device timed out and shut down. `link` and every test turn the timeout off, but a T4 test that was interrupted can leave it on. |
| T1.1 cells read `0x8000` | ADC not started, or the cell count is wrong. Check `stat`: `ACTIVE_CELL` should be `0x02`. |
| T2 GPIOs show `--` | Thermistor open or shorted, J4/J5 not fitted, or TSREF off |
| T5.1 `INVALID_CBCONF` | Two selected cells are adjacent, or more than 8 cells are selected |
| Menu doesn't respond | Your terminal must send CR on Enter. Check the UART4 wiring: PC10 → adapter RX, PC11 → adapter TX. |

---

## Part 2: How the code works

### 2.1 Layers

```
Tests/bq_test.c        test plan, menu, pass/fail checks
        │
Core/Src/bq.c          BQ79616 protocol: frames, CRC, wake ping, auto-addressing
        │
Embedded-Sharepoint    UART.c (send/receive queues, interrupts), printf, HAL
        │
USART3 (PB10/PB11) ──── BQ79616 base device ──── daisy chain ──── stack devices
UART4  (PC10/PC11) ──── debug console (printf, menu input)
```

Everything runs in one FreeRTOS task, `Task_BQ_Test` (priority idle+3, stack of 1024 words). A FreeRTOS software timer toggles the heartbeat LED, so it keeps blinking while the menu waits for input.

### 2.2 UART link (`bq_init`)

- USART3 runs at 1 Mbps, 8N1, 16× oversampling, on the PCLK1 clock.
- **PB10 (TX)** is driven high *before* it's switched over to the UART, so the BQ never sees a false low pulse during setup.
- **PB11 (RX)** has a pull-up, because the BQ's TX floats in SHUTDOWN and a floating line would produce garbage bytes.
- **Overrun detection is turned off.** In the HAL, an overrun stops interrupt-driven reception, and nothing in `UART.c` restarts it, so the link would go silent for good. With overrun detection off, a lost byte simply makes that reply fail its CRC or time out.
- The **hardware CRC unit** is set up for the BQ's CRC (CRC-16-IBM: polynomial 0x8005, start value 0xFFFF, bits reflected on input and output). This matches the datasheet's worked example: the frame `80 00 02 0F 0B` gives CRC bytes `C0 29`, low byte first.
- The **DWT cycle counter** is started here. The driver uses it to time the wake ping, and the test uses it for timing measurements.

Received bytes go into the Sharepoint UART receive queue, which holds 128 bytes and is filled one byte per interrupt. `bq_read` takes bytes from that queue, waiting at most `BQ_RESP_TIMEOUT_MS` (10 ms) for each one.

### 2.3 Frames (`bq_write`, `bq_read`)

**Command frame** (datasheet 6.3.6.1.1):

```
INIT | [DEV ADR] | REG ADR hi | REG ADR lo | DATA (1-8 bytes) | CRC lo | CRC hi
INIT = 0x80 | REQ_TYPE << 4 | (data bytes - 1)
```

- `DEV ADR` is included only for single-device reads and writes.
- For a read, the one DATA byte holds (number of bytes to return − 1).

**Reply frame**, one per responding device:

```
INIT (= bytes - 1) | DEV ADR | REG ADR hi | REG ADR lo | DATA | CRC lo | CRC hi
```

Before sending a read, `bq_read` throws away any stale bytes in the receive queue. For each expected reply it then:

1. receives exactly `len + 6` bytes,
2. checks the CRC over the whole frame (the result is 0 for a good frame),
3. checks the INIT byte and register address, plus the device address for single reads,
4. copies the data out and, if asked, records which device sent it.

**Which devices answer, and in what order:**

- A **single** read gets one reply.
- A **stack** read gets one reply from each stack device, but not the base.
- A **broadcast** read gets one reply from every device.
- In stack and broadcast reads, the **top of stack replies first** and the base replies last (6.3.6.1.1.2.2).

### 2.4 Waking and shutting down

- **`bq_wake_stack(n)`:** a WAKE ping is the BQ's RX line held low for 2.25 ms (the datasheet allows 2–2.5 ms, tHLD_WAKE). The driver makes it like this:
  1. Switches PB10 from the UART to a plain output, already driven high.
  2. Waits 10 ms with the line idle.
  3. Inside a FreeRTOS critical section, drives the line low for exactly 2.25 ms, timed with the DWT cycle counter, then releases it high.
  4. Hands PB10 back to the UART.
  5. Waits 30 ms for the device to start up and settle (tSU(WAKE_SHUT) ≤ 10 ms, plus 4 ms for the ADC front end), plus 10 ms more for each extra device in the chain.

  Each device passes a WAKE signal on to the next one up the chain. A WAKE also resets a device that's already ACTIVE (6.4.2), so this function doubles as a reset.
- **`bq_shut()`:** sends a broadcast write of `CONTROL1[GOTO_SHUTDOWN]`, then waits tSHTDN (20 ms).

### 2.5 Auto-addressing (`bq_auto_address(n)`)

This follows the datasheet's procedure (Table 6-19):

1. **Dummy write:** a broadcast write of zeros to `OTP_ECC_DATAIN1..8`. This synchronizes the daisy-chain link timing in the write direction.
2. A broadcast write to set `CONTROL1[ADDR_WR]`.
3. Broadcast writes of `DIR0_ADDR` = 0, 1, …, n−1. Each device keeps the first address it receives and passes the rest on up the chain.
4. Sets which device is base and which is top of stack (`COMM_CTRL`):
   - A broadcast write makes every device a stack device (`STACK_DEV`).
   - A single write makes device 0 the base. If it's the only device, it's also the top of stack.
   - With more than one device, a single write sets `STACK_DEV` | `TOP_STACK` on device n−1.
5. **Dummy read:** a broadcast read of the same registers, synchronizing the read direction. Its reply is expected to be incomplete and is thrown away.
6. Clears the communication faults that the dummy read and write are expected to leave behind (`FAULT_RST1/2` = 0xFF).
7. Reads back `DIR0_ADDR` from every device with a broadcast read. Each reply must come from the right device and contain the right address, top of stack first.

In the test, `link_up()` runs a wake followed by auto-addressing. It then turns the communication timeout off on every device, so the chip can't shut itself down while the menu sits idle, and checks PARTID on every device. `ensure_link()` re-links only if a quick PARTID read of device 0 fails.

### 2.6 Measurements and conversions

**ADC setup** (`adc_start`, broadcast so it applies to every device):

- `ACTIVE_CELL` = `0x02` (8S).
- `GPIO_CONF1..4` = `0x12` (all GPIOs as ADC inputs).
- `CONTROL2[TSREF_EN]` = 1, then a 10 ms wait for TSREF to ramp up.
- `ADC_CONF1` sets the filter to 26 Hz, or turns it off for T5.6.
- `ADC_CTRL1` = MAIN_GO plus the mode: continuous, or single-shot for T5.6.
- Waits for `ADC_STAT1[DRDY_MAIN_ADC]`.

All of these settings must be in place before MAIN_GO is written, because the ADC only reads them at start.

**One read gets everything.** The result registers from `VCELL8_HI` (0x578) to `GPIO8_LO` (0x59D) form one 38-byte block containing VCELL8…VCELL1, BUSBAR, TSREF and GPIO1…GPIO8. One read of that block is a full "poll cycle".

**Conversions** (datasheet Table 6-1). Every result is a signed 16-bit number. `0x8000` means "not converted yet".

| Value | Scale |
|---|---|
| VCELL | 190.73 µV per count |
| GPIO | 152.59 µV per count |
| TSREF | 169.54 µV per count |

**Thermistor temperature.** Each GPIO input sits on a divider: TSREF → pull-up resistor → GPIO → NTC thermistor → ground. The firmware computes:

```
ratio = V_GPIO / V_TSREF
R_ntc = R_pullup * ratio / (1 - ratio)
T(°C) = 1 / (1/298.15 + ln(R_ntc / R25) / Beta) - 273.15
```

The build doesn't link the maths library, so the firmware computes `ln` itself (`ln_f`, accurate to about 1e-6). A ratio outside 0.1%–99.9% is treated as an open or shorted thermistor.

### 2.7 Balancing (device 0)

- **Timers:** `CB_CELL16_CTRL` … `CB_CELL1_CTRL` are consecutive registers (0x318–0x327), so all 16 timers are written with two 8-byte writes. A timer code of `0x04` is 300 s; `0x00` means off.
- **Starting:** `BAL_CTRL2` = `BAL_GO` starts balancing in **manual** mode. The FETs follow the timers exactly, with no automatic odd/even cycling.
- **Pause and resume:** `BAL_CTRL2` = `CB_PAUSE` pauses and `0x00` resumes. Neither needs BAL_GO, because `CB_PAUSE` takes effect immediately, while the other settings are only read at BAL_GO.
- **Remaining time:** writing `BAL_CTRL3` = (cell−1)<<1 | `BAL_TIME_GO` latches that cell's remaining time into `BAL_TIME`. The firmware reads it as 5 s steps, or 5 min steps when bit 7 is set. A reading of `0x7F` means "held stopped".
- **Latency timing (T5.5):** the firmware records the cycle counter when it sends the command. It then watches the HAL UART state until sending finishes, and polls `BAL_STAT` until `CB_INPAUSE` changes.

### 2.8 Communication timeout tests

- **Timeout setting:** `COMM_TIMEOUT_CONF` = `CTL_ACT` (0x08, which makes the action SHUTDOWN) | `CTL_TIME` code.
- **Detecting a reset:** the firmware can't observe a device's power state directly. Instead, it writes `0xA5` into a scratch register (`OTP_ECC_DATAIN9`, 0x34B) on every device. That register clears to 0 on any reset, and auto-addressing doesn't touch it. So:
  - a marker still at `0xA5` means the device was never reset (T4.4),
  - a marker at `0x00` means it was (T4.1, T4.3).
- **Timing bracket:** the probe at 80% shows the device was still alive. The read at 120% getting no reply shows it had shut down. That places the shutdown between the two.
- **T4.2 Phase A** sets the timeout only on the top device, so the base stays up and can be used to check whether the top device answers.

### 2.9 Datasheet sections used

| Topic | Section |
|---|---|
| Frame format, CRC | 6.3.6.1.1 |
| Stack and broadcast reads | 6.3.6.1.1.2.2 |
| Starting communication, auto-addressing | 6.3.6.1.3, Table 6-19 |
| Power modes, resets, pings and tones | 6.4.1, 6.4.2, 6.4.3 |
| Main ADC and conversions | 6.3.1, Table 6-1 |
| Cell balancing, pause, stop | 6.3.3 |
| Register descriptions | 6.5 |
| Timing (tHLD_WAKE, tSU, tSHTDN, tTSREF_ON) | 5.6 |
| TSREF and ADC resolution specs | 5.5 |

---

## Known limitations

- **Not yet verified on hardware.** All of the logic above comes from the datasheet and hasn't been checked against an EVM.
- **Thermistor values are assumed.** The pull-up, R25 and Beta values haven't been checked against the EVM's parts list.
- **BAL_TIME scaling is an interpretation.** The datasheet says "5 sec step", and the firmware reads that as each count being 5 s. If T5.3 or T5.4 report odd times, check this first.
- **Unselected channels in T5.1:** the datasheet doesn't say what `BAL_TIME` reports for a cell that isn't balancing, so the test accepts either 0 or the "held stopped" value.
- **Receive queue size:** the Sharepoint UART receive queue holds 128 bytes. The biggest read in these tests is 88 bytes (two devices × 44), which fits. Reading close to the 128-byte maximum in one go could overflow the queue if the task were slowed down.
- **Unimplemented driver functions:** `bq_config()`, `bq_sleep()` and `bq_rouse()` are declared in `bq.h` but not implemented. None of the tests use them.
- **Build dependency outside this repo:** the build needs `HAL_CRC_MODULE_ENABLED` in the Embedded-Sharepoint submodule's `stm32g4xx_hal_conf.h`. That change is currently uncommitted.
