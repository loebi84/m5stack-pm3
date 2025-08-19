#pragma once
#include <stdint.h>

// VSPI pins (M5Stack Basic)
static const uint8_t PM3_SPI_SCK  = 18;
static const uint8_t PM3_SPI_MISO = 19;
static const uint8_t PM3_SPI_MOSI = 23;

// Chip selects
static const uint8_t PM3_SD_CS  = 4;  // TF-Slot
static const uint8_t PM3_USB_CS = 5;  // MAX3421E

// OPTIONAL: only variable aliases for existing code (no macros)
static const uint8_t SD_CS_CONST      = PM3_SD_CS;
static const uint8_t USB_MAX_CS_CONST = PM3_USB_CS;
