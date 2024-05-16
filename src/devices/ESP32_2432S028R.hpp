#pragma once
// display: ILI9341
//     bus: SPI
//
// implements device using TFT_eSPI and XPT2046_Bitbang

#include "../device.hpp"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Bitbang.h>

class ESP32_2432S028R final : public device {
  TFT_eSPI display{};
  SPIClass spi3{SPI3_HOST};
  XPT2046_Bitbang touch_screen{TOUCH_MOSI, TOUCH_MISO, TOUCH_SCK, TOUCH_CS};
  TouchPoint touch{};

public:
  auto init() -> void override {
    // initiate display
    display.init();
    display.initDMA(true);
    display.setRotation(display_orientation == TFT_ORIENTATION ? 0 : 1);
    display.setAddrWindow(0, 0, display_width, display_height);

    spi3.begin(SD_SCK, SD_MISO, SD_MOSI);
    if (!SD.begin(SD_CS, spi3, 10000000)) {
      printf("* no SD card\n");
    }

    touch_screen.begin();
  }

  auto display_is_touched() -> bool override {
    touch = touch_screen.getTouch();
    return touch.zRaw != 0;
  }

  auto display_get_touch(uint16_t &x, uint16_t &y,
                         uint8_t &pressure) -> void override {
    // flip x and y for display orientation
    x = 4096 - touch.yRaw;
    y = touch.xRaw;
    pressure = touch.zRaw;
  }

  auto dma_write_bytes(uint8_t const *data, uint32_t len) -> void override {
    // note. TFT_eSPI requires non-const data in case bytes are swapped
    // note. pushPixelsDMA(...) waits for previous transaction to complete
    display.pushPixelsDMA(
        reinterpret_cast<uint16_t *>(const_cast<uint8_t *>(data)),
        len / sizeof(uint16_t));
  }

  auto dma_is_busy() -> bool override { return display.dmaBusy(); }

  auto dma_wait_for_completion() -> void override { return display.dmaWait(); }

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
};