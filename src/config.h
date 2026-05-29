#ifndef CONFIG_H
#define CONFIG_H

#define SCHEDULER_SOURCE millis()

// SPI Pins für Waveshare ESP32-S3 Touch LCD 2
#define PIN_MISO      40
#define PIN_MOSI      38
#define PIN_SCLK      39
#define PIN_LCD_CS    45
#define PIN_LCD_DC    42
#define PIN_LCD_RST   -1   // Kein Reset-Pin
#define PIN_BACKLIGHT 1

#endif