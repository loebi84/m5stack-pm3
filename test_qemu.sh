#!/bin/bash
# QEMU ESP32 Test Script
# This script runs the flash_image.bin (or creates one from firmware.bin) in QEMU ESP32 emulator for local testing

set -e

FIRMWARE_PATH="${1:-.pio/build/m5stack_basic/firmware.bin}"
TIMEOUT="${2:-30}"
DOCKER_IMAGE="espressif/idf:latest"
QEMU_BIN="/opt/esp/tools/qemu-xtensa/esp_develop_9.2.2_20250817/qemu/bin/qemu-system-xtensa"

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

# Determine if we have a flash_image.bin or need to create one
FLASH_IMAGE=""
FIRMWARE_DIR=$(dirname "$FIRMWARE_PATH")
FIRMWARE_NAME=$(basename "$FIRMWARE_PATH")

if [ "$FIRMWARE_NAME" = "flash_image.bin" ]; then
    FLASH_IMAGE="$FIRMWARE_PATH"
    echo "Using existing flash image"
elif [ -f "$FIRMWARE_DIR/flash_image.bin" ]; then
    FLASH_IMAGE="$FIRMWARE_DIR/flash_image.bin"
    echo "Using flash image from build directory: $FLASH_IMAGE"
else
    echo "Creating merged flash image from firmware components..."
    
    # Check for bootloader and partition table
    BOOTLOADER="$FIRMWARE_DIR/bootloader.bin"
    PARTITIONS="$FIRMWARE_DIR/partitions.bin"
    
    if [ ! -f "$BOOTLOADER" ]; then
        echo "Warning: bootloader.bin not found at $BOOTLOADER"
        echo "Attempting to use firmware.bin directly (may not boot properly)"
        FLASH_IMAGE="$FIRMWARE_PATH"
    elif [ ! -f "$PARTITIONS" ]; then
        echo "Warning: partitions.bin not found at $PARTITIONS"
        echo "Attempting to use firmware.bin directly (may not boot properly)"
        FLASH_IMAGE="$FIRMWARE_PATH"
    else
        # Create merged flash image using esptool
        echo "Found bootloader and partition table, creating merged image..."
        
        # Check if esptool is available
        if ! command -v esptool.py &> /dev/null; then
            echo "Installing esptool..."
            pip install esptool --user
        fi
        
        FLASH_IMAGE="$FIRMWARE_DIR/flash_image.bin"
        
        esptool.py --chip esp32 merge_bin \
            -o "$FLASH_IMAGE" \
            --flash_mode dio \
            --flash_size 4MB \
            0x1000 "$BOOTLOADER" \
            0x8000 "$PARTITIONS" \
            0x10000 "$FIRMWARE_PATH"
        
        echo "Created flash image: $(ls -lh $FLASH_IMAGE | awk '{print $5}')"
    fi
fi

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

# QEMU needs write access to the flash image, so make a working copy
WORK_FLASH="qemu_logs/flash_image_work_$(date +%Y%m%d_%H%M%S).bin"
cp "$FLASH_IMAGE" "$WORK_FLASH"
echo "Created working copy: $WORK_FLASH"

# Run QEMU with timeout
timeout ${TIMEOUT}s docker run --rm \
  -v "$(pwd)/qemu_logs":/qemu_logs \
  $DOCKER_IMAGE \
  bash -c "$QEMU_BIN \
    -nographic \
    -M esp32 \
    -m 4M \
    -drive file=/qemu_logs/$(basename "$WORK_FLASH"),if=mtd,format=raw \
    -global driver=esp32.gpio,property=strap_mode,value=0x0f \
    -serial file:/qemu_logs/$(basename "$LOG_FILE")" || true

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
