#pragma once

// --- Board pins (M5Stack Basic, VSPI) ---
static const int PM3_SPI_SCK  = 18;
static const int PM3_SPI_MISO = 19;
static const int PM3_SPI_MOSI = 23;

// --- Chip selects ---
static const int PM3_SD_CS    = 4;   // TF-Slot
static const int PM3_USB_CS   = 5;   // MAX3421E (USB Host Shield)

// --- Backward-compatible aliases used im Projekt ---
#define SD_CS       PM3_SD_CS
#define USB_MAX_CS  PM3_USB_CS

// --- Aliasse, die UsbHostProxmark.hpp erwartet ---
#define USB_SPI_SCK  PM3_SPI_SCK
#define USB_SPI_MISO PM3_SPI_MISO
#define USB_SPI_MOSI PM3_SPI_MOSI
