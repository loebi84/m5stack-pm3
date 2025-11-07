# QEMU ESP32 Emulation Guide

This document describes how to test the M5Stack PM3 firmware using QEMU ESP32 emulator, both in CI/CD and locally.

## Overview

QEMU (Quick Emulator) can emulate an ESP32 microcontroller, allowing basic firmware testing without physical hardware. This is useful for:

- **Detecting boot failures** - Catch firmware crashes during initialization
- **Validating builds** - Ensure firmware binary is valid and bootable
- **CI/CD Integration** - Automated testing after each build
- **Early debugging** - Find issues before deploying to hardware

## Limitations

⚠️ **Important**: QEMU ESP32 has limited hardware simulation:

- ❌ **No M5Stack display** - Display functions will not work
- ❌ **No physical buttons** - Button inputs cannot be tested
- ❌ **No USB Host (MAX3421E)** - USB communication is not simulated
- ❌ **No SD card** - File system operations are limited
- ❌ **No WiFi/Bluetooth** - Network features are not available

**What CAN be tested:**
- ✅ Firmware boot sequence
- ✅ Basic initialization code
- ✅ CPU and memory operations
- ✅ Serial/UART output
- ✅ Detection of crashes and panics

## GitHub Actions Integration

The QEMU test workflow runs automatically after successful firmware builds.

### Workflow File

Location: `.github/workflows/qemu-test.yml`

### Trigger

The QEMU test workflow triggers automatically when:
- The "Build firmware (PlatformIO)" workflow completes successfully
- Manually via workflow_dispatch in GitHub Actions UI

### Viewing Results

1. Go to your repository on GitHub
2. Click **Actions** tab
3. Select **QEMU ESP32 Test** workflow
4. View the latest run
5. Check the "Analyze QEMU output" step for serial output
6. Download the "qemu-logs" artifact for full logs

## Local Testing with Docker

### Quick Start Script

The repository includes a helper script `test_qemu.sh` for easy local testing:

```bash
# Test with default firmware location
./test_qemu.sh

# Test with custom firmware path
./test_qemu.sh firmware.bin

# Test with custom timeout (60 seconds)
./test_qemu.sh firmware.bin 60
```

The script will:
- Check for the firmware file
- Pull the QEMU Docker image if needed
- Run the emulation with timeout
- Save logs to `qemu_logs/` directory
- Analyze output for boot indicators and errors

### Manual Docker Testing

### Prerequisites

- Docker installed on your system
- A compiled `firmware.bin` file

### Step 1: Get the QEMU Docker Image

```bash
docker pull espressif/qemu:esp-develop-20220919
```

### Step 2: Run Your Firmware

From your project directory:

```bash
# If you have firmware.bin in .pio/build/m5stack_basic/
docker run --rm \
  -v $(pwd)/.pio/build/m5stack_basic/firmware.bin:/firmware.bin:ro \
  espressif/qemu:esp-develop-20220919 \
  /opt/qemu/bin/qemu-system-xtensa \
  -nographic \
  -machine esp32 \
  -drive file=/firmware.bin,if=mtd,format=raw \
  -serial stdio
```

Or if you have `firmware.bin` in the current directory:

```bash
docker run --rm \
  -v $(pwd)/firmware.bin:/firmware.bin:ro \
  espressif/qemu:esp-develop-20220919 \
  /opt/qemu/bin/qemu-system-xtensa \
  -nographic \
  -machine esp32 \
  -drive file=/firmware.bin,if=mtd,format=raw \
  -serial stdio
```

### Step 3: Stop the Emulation

Press `Ctrl+C` to stop QEMU (or use `timeout` command to auto-stop after N seconds).

### Example with Timeout

```bash
timeout 30s docker run --rm \
  -v $(pwd)/firmware.bin:/firmware.bin:ro \
  espressif/qemu:esp-develop-20220919 \
  /opt/qemu/bin/qemu-system-xtensa \
  -nographic \
  -machine esp32 \
  -drive file=/firmware.bin,if=mtd,format=raw \
  -serial stdio
```

This will run for 30 seconds and automatically stop.

## Local Testing without Docker

### Prerequisites

- Install QEMU with ESP32 support
- Build QEMU from Espressif's fork: https://github.com/espressif/qemu

### Build QEMU (Linux/macOS)

```bash
git clone https://github.com/espressif/qemu.git
cd qemu
./configure --target-list=xtensa-softmmu \
    --enable-gcrypt \
    --enable-debug \
    --disable-strip --disable-user \
    --disable-capstone --disable-vnc \
    --disable-sdl --disable-gtk
make -j$(nproc)
```

**Note:** The `--enable-sanitizers` flag can be added for development but may cause build issues on some systems. Omit it for a standard build.

### Run Firmware

```bash
./build/qemu-system-xtensa \
  -nographic \
  -machine esp32 \
  -drive file=/path/to/firmware.bin,if=mtd,format=raw \
  -serial stdio
```

## Interpreting Results

### Successful Boot Indicators

Look for messages like:
```
ESP-ROM:esp32...
ets Jun  8 2016 00:22:57
...
[any custom setup messages from your firmware]
```

### Common "Expected" Errors

Due to missing hardware simulation, you may see errors like:
- `Guru Meditation Error` - Often caused by missing hardware peripherals
- `Task watchdog` - Can occur if hardware initialization hangs
- `abort()` called - May happen when peripherals don't respond

**These errors are expected** in QEMU and indicate the firmware tried to access hardware that isn't simulated.

### Real Problems to Watch For

- **No output at all** - Firmware may not be loading correctly
- **Immediate crash** - Boot loader or initialization issue
- **Invalid instruction** - Firmware compiled for wrong architecture

## Advanced QEMU Options

### Enable GDB Debugging

```bash
docker run --rm -p 1234:1234 \
  -v $(pwd)/firmware.bin:/firmware.bin:ro \
  espressif/qemu:esp-develop-20220919 \
  /opt/qemu/bin/qemu-system-xtensa \
  -nographic \
  -machine esp32 \
  -drive file=/firmware.bin,if=mtd,format=raw \
  -serial stdio \
  -s -S
```

Then connect with GDB:
```bash
xtensa-esp32-elf-gdb .pio/build/m5stack_basic/firmware.elf
(gdb) target remote localhost:1234
(gdb) continue
```

### Save Serial Output to File

```bash
docker run --rm \
  -v $(pwd)/firmware.bin:/firmware.bin:ro \
  -v $(pwd):/output \
  espressif/qemu:esp-develop-20220919 \
  /opt/qemu/bin/qemu-system-xtensa \
  -nographic \
  -machine esp32 \
  -drive file=/firmware.bin,if=mtd,format=raw \
  -serial file:/output/qemu_serial.log
```

## Troubleshooting

### Docker: Permission Denied

Make sure the firmware.bin file is readable:
```bash
chmod 644 firmware.bin
```

### No Output Visible

- Check that you're using `-serial stdio` or `-serial file:/path`
- Ensure `-nographic` flag is present
- Try adding `-d guest_errors` for debug output

### QEMU Crashes Immediately

- Verify firmware.bin is a valid binary (not empty, not corrupted)
- Check that you're using the ESP32 machine type: `-machine esp32`

## References

- [Espressif QEMU Repository](https://github.com/espressif/qemu)
- [QEMU ESP32 Docker Hub](https://hub.docker.com/r/espressif/qemu)
- [QEMU ESP32 CI Example](https://github.com/espressif/qemu/blob/master/.github/workflows/ci.yml)

## Integration with Build Process

The QEMU test is integrated into the CI/CD pipeline:

```
Build Workflow → Creates firmware.bin artifact → QEMU Test Workflow downloads artifact → Tests in emulator
```

Developers can download the firmware from successful builds and test locally before deploying to hardware.
