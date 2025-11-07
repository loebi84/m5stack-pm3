#!/usr/bin/env python3
"""
Create a minimal ESP32 test firmware for QEMU testing.
This creates a merged flash image with bootloader, partition table, and a minimal app.
"""
import struct
import os

# Create a minimal bootloader (just enough to satisfy QEMU)
# Real bootloader is ~27KB, we'll create a stub
bootloader = b'\xe9' + b'\x00' * (0x7000 - 1)  # ESP32 bootloader magic + padding

# Create a minimal partition table at 0x8000
# Format: magic(0xAA50) + type + subtype + offset + size + name + flags
partition_entry = struct.pack('<2sBBLL32sL',
    b'\xAA\x50',  # magic
    0x00,  # type: app
    0x00,  # subtype: factory
    0x10000,  # offset: 64KB
    0x100000,  # size: 1MB
    b'factory\x00' + b'\x00' * 24,  # name
    0  # flags
)
partition_table = partition_entry + b'\xFF' * (0x1000 - len(partition_entry))

# Create a minimal app binary
# ESP32 app should start with image header
app_header = b'\xe9\x00\x00\x00' + b'\x00' * 20  # Minimal image header
app = app_header + b'\x00' * (0x1000 - len(app_header))

# Create merged flash image
flash_size = 0x400000  # 4MB
flash_image = bytearray(flash_size)

# Write bootloader at 0x1000
flash_image[0x1000:0x1000 + len(bootloader)] = bootloader

# Write partition table at 0x8000
flash_image[0x8000:0x8000 + len(partition_table)] = partition_table

# Write app at 0x10000
flash_image[0x10000:0x10000 + len(app)] = app

# Save flash image
with open('flash_image.bin', 'wb') as f:
    f.write(flash_image)

print(f"Created flash_image.bin ({len(flash_image)} bytes)")
