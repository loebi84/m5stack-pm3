#!/bin/bash
# QEMU ESP32 Test Script
# This script runs the firmware.bin in QEMU ESP32 emulator for local testing

set -e

FIRMWARE_PATH="${1:-.pio/build/m5stack_basic/firmware.bin}"
TIMEOUT="${2:-30}"
DOCKER_IMAGE="espressif/qemu:esp-develop-20220919"

echo "=========================================="
echo "QEMU ESP32 Firmware Test"
echo "=========================================="
echo "Firmware: $FIRMWARE_PATH"
echo "Timeout: ${TIMEOUT}s"
echo "Docker Image: $DOCKER_IMAGE"
echo "=========================================="
echo ""

# Check if firmware exists
if [ ! -f "$FIRMWARE_PATH" ]; then
    echo "Error: Firmware file not found: $FIRMWARE_PATH"
    echo ""
    echo "Usage: $0 [firmware.bin] [timeout_seconds]"
    echo "Example: $0 firmware.bin 30"
    exit 1
fi

echo "Firmware found: $(ls -lh $FIRMWARE_PATH | awk '{print $5}')"
echo ""

# Pull Docker image if not present
echo "Checking for QEMU Docker image..."
if ! docker image inspect $DOCKER_IMAGE &>/dev/null; then
    echo "Pulling QEMU Docker image (this may take a moment)..."
    docker pull $DOCKER_IMAGE
fi
echo "Docker image ready"
echo ""

# Create output directory
mkdir -p qemu_logs
LOG_FILE="qemu_logs/qemu_test_$(date +%Y%m%d_%H%M%S).log"

echo "Starting QEMU emulation..."
echo "Output will be saved to: $LOG_FILE"
echo "Press Ctrl+C to stop early"
echo ""
echo "=========================================="

# Run QEMU with timeout
timeout ${TIMEOUT}s docker run --rm \
  -v "$(cd "$(dirname "$FIRMWARE_PATH")" && pwd)/$(basename "$FIRMWARE_PATH")":/firmware.bin:ro \
  $DOCKER_IMAGE \
  /opt/qemu/bin/qemu-system-xtensa \
  -nographic \
  -machine esp32 \
  -drive file=/firmware.bin,if=mtd,format=raw \
  -serial stdio \
  2>&1 | tee $LOG_FILE || true

echo ""
echo "=========================================="
echo "QEMU Test Complete"
echo "=========================================="
echo ""

# Analyze output
echo "Analyzing output..."
echo ""

if [ -f "$LOG_FILE" ]; then
    # Check for boot indicators
    if grep -qi "ESP-ROM\|ets Jun\|boot" $LOG_FILE; then
        echo "✓ Boot sequence detected"
    else
        echo "⚠ No clear boot sequence found"
    fi
    
    # Check for errors
    if grep -qi "panic\|abort\|crash\|fatal error" $LOG_FILE; then
        echo "⚠ Errors/crashes detected (may be expected due to missing hardware)"
    fi
    
    # Check for initialization
    if grep -qi "setup\|initialized\|ready\|begin" $LOG_FILE; then
        echo "✓ Initialization messages found"
    fi
    
    echo ""
    echo "Log saved to: $LOG_FILE"
    echo ""
    echo "Note: QEMU has limited hardware simulation."
    echo "Missing peripherals (display, buttons, USB, SD) may cause expected errors."
else
    echo "⚠ No log file generated"
fi

echo ""
echo "For more information, see QEMU_EMULATION.md"
