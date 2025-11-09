# GitHub Actions for PlatformIO (ESP32 / M5Stack)

This repo is prepped with three workflows:

- `.github/workflows/build.yml` — builds on every push/PR to `main`, uploads `firmware.bin` as an Artifact
- `.github/workflows/qemu-test.yml` — tests firmware in QEMU ESP32 emulator after successful builds
- `.github/workflows/release.yml` — builds when you push a tag `v*` and attaches `firmware.bin` to a GitHub Release

## Usage

1. Place your PlatformIO project at repo root:
   - `platformio.ini`
   - `src/`, `include/`, etc.
2. Commit & push to `main`.
3. Go to **Actions** → `build-esp32` → download artifact **firmware** (contains `firmware.bin`).

### Selecting a specific environment
If your `platformio.ini` has a specific env (e.g. `m5stack_basic`), edit the workflow step:
```yaml
- name: Build (env)
  run: pio run -e m5stack_basic
```
In `release.yml` change the same step.

### OTA to M5 (from any machine / iPhone)
Your firmware exposes a simple OTA endpoint at `/_update` or `/update`.
```bash
curl -F "firmware=@firmware.bin" http://<M5-IP>/update
```

### QEMU Testing
The `qemu-test.yml` workflow automatically tests the firmware in a QEMU ESP32 emulator after each successful build. This helps catch:
- Boot failures and crashes
- Initialization errors
- Invalid firmware binaries

**Note:** QEMU has limited hardware simulation (no display, buttons, USB, SD card). See `QEMU_EMULATION.md` for details on local testing.

### Notes
- All workflows install PlatformIO in the CI runner (Ubuntu).
- Artifacts are kept by GitHub for a limited time. Use `release.yml` for persistent release files.
- QEMU logs are uploaded as artifacts for debugging.
