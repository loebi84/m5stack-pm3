# GitHub Copilot Instructions - M5Stack PM3 Project

## Project Overview
This is an embedded firmware project for M5Stack devices that interfaces with Proxmark3 hardware via USB. The project uses PlatformIO with the Arduino framework for ESP32.

## Coding Standards

### Language & Framework
- Primary language: C++ (Arduino framework)
- Target platform: ESP32 (M5Stack Core)
- Build system: PlatformIO
- C++ standard: gnu++17

### Code Style
- Use camelCase for function names (e.g., `sendProxmarkAscii`, `pumpProxmarkToDisplay`)
- Use PascalCase for class names (e.g., `UsbHostProxmark`, `WebUpdateServer`)
- Keep header files in the `include/` directory with `.hpp` or `.h` extensions
- Main application code in `src/main.cpp`
- Prefer inline implementations for small classes in header files
- Use descriptive variable names, avoid single-letter names except for loop counters

### Memory & Performance
- Be mindful of stack usage - this is embedded firmware with limited RAM
- Use `const char*` for string literals to save RAM
- Prefer stack allocation over heap when possible
- Use `std::vector` sparingly, consider fixed-size buffers for embedded constraints
- Always check buffer boundaries to prevent overflows

## Build & Testing

### Building
```bash
# Install PlatformIO
pip install platformio

# Build the firmware
pio run -e m5stack_basic

# Build with verbose output
pio run -e m5stack_basic -v
```

### Testing
- The project currently has no automated tests
- Manual testing is done on physical M5Stack hardware with Proxmark3 device
- Test firmware updates via OTA and SD card methods
- Verify USB communication with actual Proxmark3 hardware

## Architecture

### Key Components
- **UsbHostProxmark**: Handles USB CDC communication with Proxmark3
- **WebUpdateServer**: Provides OTA firmware updates via web interface
- **UsbDebugView**: Debug interface for USB communication
- **HexTools**: Utilities for hex/byte conversions

### Dependencies
- M5Unified (v0.1.13+): Core M5Stack library
- ArduinoJson (v7): JSON parsing for web interface
- AsyncTCP-esphome: Async networking
- ESPAsyncWebServer-esphome: Web server for OTA
- USB_Host_Shield_2.0: USB host functionality

### File Organization
```
/src/main.cpp           - Main application entry point
/include/*.hpp          - Header files with class definitions
/platformio.ini         - Build configuration
/.github/workflows/     - CI/CD pipelines
```

## Security Guidelines

### Embedded Security
- Never hardcode WiFi credentials - use configuration or runtime input
- Validate all external inputs (USB data, web requests) before processing
- Check buffer sizes before copying data to prevent overflows
- Always use bounds checking when accessing arrays
- Clear sensitive data from memory after use

### OTA Updates
- OTA updates are served via web interface - ensure proper authentication in production
- SD card updates use `/firmware.bin` file - validate file integrity before flashing
- Always verify Update.begin() success before writing firmware data

### USB Communication
- Validate USB device descriptors before initialization
- Check data lengths from USB reads before processing
- Handle USB disconnection gracefully without crashing

## Documentation

### Code Comments
- Add comments for complex algorithms or non-obvious logic
- Document public class methods and their parameters
- Explain hardware-specific interactions (USB, SPI, display)
- Keep comments concise and up-to-date with code changes

### Commit Messages
- Use clear, descriptive commit messages
- Reference issue numbers when applicable
- Keep commits focused on single logical changes

## Project-Specific Guidelines

### Display Output
- Use `M5.Display.println()` for output to the M5Stack screen
- Color coding: Yellow for sent commands, Green for received data, Red for errors
- Use TFT_YELLOW, TFT_GREEN, TFT_RED, TFT_BLACK constants for colors

### USB Communication
- Always check `pm3.ready()` before sending data to Proxmark3
- Append newline (`\n`) to ASCII commands if not present
- Handle both ASCII and raw hex command formats
- Use `pumpProxmarkToDisplay()` regularly to read and display responses

### Error Handling
- Display error messages with `[ERR]` prefix
- Check return values from SD card, Update, and USB operations
- Provide clear error messages to users on the display
- Don't crash on errors - handle gracefully and inform the user

### Web Interface
- Raw hex commands via `/send?hex=...` endpoint
- OTA update UI accessible via web server
- Keep web responses lightweight for embedded device

## CI/CD

### GitHub Actions
- Build workflow runs on push/PR to main branch
- Uses PlatformIO to build firmware
- Produces `firmware.bin` artifact
- Release workflow handles versioning and artifact publishing

### Build Requirements
- Python 3.x
- PlatformIO installed via pip
- No additional system dependencies required

---

*These instructions help GitHub Copilot understand the project structure and coding conventions. Update as the project evolves.*
