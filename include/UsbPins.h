#pragma once

// Falls irgendwo alte Makros definiert waren: weg damit.
#ifdef SPI_SCK
  #undef SPI_SCK
#endif
#ifdef SPI_MISO
  #undef SPI_MISO
#endif
#ifdef SPI_MOSI
  #undef SPI_MOSI
#endif
#ifdef SD_CS
  #undef SD_CS
#endif
#ifdef USB_MAX_CS
  #undef USB_MAX_CS
#endif
#ifdef USB_SPI_SCK
  #undef USB_SPI_SCK
#endif
#ifdef USB_SPI_MISO
  #undef USB_SPI_MISO
#endif
#ifdef USB_SPI_MOSI
  #undef USB_SPI_MOSI
#endif

// --- Board pins (M5Stack Basic, VSPI) ---
static constexpr int PM3_SPI_SCK  = 18;
static constexpr int PM3_SPI_MISO = 19;
static constexpr int PM3_SPI_MOSI = 23;

// --- Chip selects ---
static constexpr int PM3_SD_CS    = 4;   // TF-Slot
static constexpr int PM3_USB_CS   = 5;   // MAX3421E (USB Host Shield)

// --- Projekt-Kompatibilität (keine Makros nötig, aber wir liefern beides) ---
static constexpr int SD_CS_CONST      = PM3_SD_CS;
static constexpr int USB_MAX_CS_CONST = PM3_USB_CS;

// Falls Altcode Makros erwartet:
#define SD_CS       SD_CS_CONST
#define USB_MAX_CS  USB_MAX_CS_CONST

// --- Aliasse, die UsbHostProxmark.hpp / UHS-Wrapper erwarten ---
#define USB_SPI_SCK  PM3_SPI_SCK
#define USB_SPI_MISO PM3_SPI_MISO
#define USB_SPI_MOSI PM3_SPI_MOSI
