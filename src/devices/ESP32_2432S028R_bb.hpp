#pragma once
// display: ILI9341
//     bus: SPI
//
// implements device using bb_spi_lcd

#include "../device.hpp"
#include "hal/spi_types.h"
#include <SPI.h>
#include <SPIFFS.h>
#include <bb_spi_lcd.h>

class ESP32_2432S028R final : public device {
  SPIClass spi3{SPI3_HOST};
  BB_SPI_LCD display{};
  TOUCHINFO touch_info{};

public:
  auto init() -> void override {
    // initiate display
    display.begin(DISPLAY_CYD);
    display.rtInit();
    display.setRotation(display_orientation == TFT_ORIENTATION ? 0 : 1);
    display.setAddrWindow(0, 0, display_width, display_height);

    // spi3.begin(SD_SCK, SD_MISO, SD_MOSI);
    // if (!SD.begin(SD_CS, spi3)) {
    //   printf("* no SD card\n");
    // }

    if (!SPIFFS.begin()) {
      Serial.println("* no SPIFFS");
    }
  }

  auto display_is_touched() -> bool override {
    return display.rtReadTouch(&touch_info);
  }

  auto display_get_touch(uint16_t &x, uint16_t &y,
                         uint8_t &pressure) -> void override {
    x = (TFT_WIDTH - touch_info.x[0]) * 4096 / TFT_WIDTH;
    y = touch_info.y[0] * 4096 / TFT_HEIGHT;
    pressure = touch_info.pressure[0];
    // ESP_LOGI("b", "x=%d   y=%d   p=%d", x, y, pressure);
  }

  auto dma_write_bytes(uint8_t const *data, uint32_t len) -> void override {
    // note. pushPixels(...) waits for previous transaction to complete
    display.pushPixels(
        reinterpret_cast<uint16_t *>(const_cast<uint8_t *>(data)),
        len / sizeof(uint16_t), DRAW_TO_LCD | DRAW_WITH_DMA);
  }

  auto dma_is_busy() -> bool override { return spilcdIsDMABusy(); }

  auto dma_wait_for_completion() -> void override {
    return;
    // spilcdWaitDMA();
  }

  auto sd_read(char const *path, char *buf, int buf_len) -> int override {
    File file = SD.open(path);
    if (!file) {
      return -1;
    }
    const size_t n = file.read((uint8_t *)buf, buf_len);
    file.close();
    return n;
  }

  auto sd_write(char const *path, char const *buf,
                int buf_len) -> bool override {
    File file = SD.open(path, FILE_WRITE);
    if (!file) {
      return false;
    }
    const size_t n = file.write((uint8_t *)buf, buf_len);
    const bool ok = n == buf_len;
    file.close();
    return ok;
  }

  auto spiffs_read(char const *path, char *buf, int buf_len) -> int override {
    File file = SPIFFS.open(path);
    if (!file) {
      return -1;
    }

    const size_t n = file.readBytes(buf, buf_len);

    file.close();

    return n;
  }
};