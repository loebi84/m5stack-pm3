
# PM3 M5Stack Final
M5Stack Basic + M5 USB Host (MAX3421E) -> Proxmark3 Easy (USB CDC).

- Menu A/B/C: navigate & run common PM3 commands
- ASCII send with newline; responses printed as readable ASCII on display
- RAW Hex via web `/send?hex=...`
- USB Debug screen (DTR/RTS, LineCoding, Snapshots, Descriptor dump)
- OTA web UI and manual SD update (no auto-boot flashing)

Open folder in PlatformIO and upload to `m5stack-core-esp32`.

## Testing

### QEMU Emulation
Test firmware virtually with QEMU ESP32 emulator:
```bash
./test_qemu.sh firmware.bin
```
See [QEMU_EMULATION.md](QEMU_EMULATION.md) for detailed instructions.

### CI/CD
- Automated builds on every push (see [README_GHA.md](README_GHA.md))
- Automatic QEMU testing after successful builds
- Firmware artifacts available for download

# m5stack-pm3
