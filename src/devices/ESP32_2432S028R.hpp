#pragma once
// display: ILI9341
//     bus: SPI

#include "../device.hpp"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

class ESP32_2432S028R final : public device {
  TFT_eSPI display{};
  SPIClass spi3{SPI3_HOST};
  XPT2046_Touchscreen touch_screen{TOUCH_CS, TOUCH_IRQ};

public:
  auto init() -> void override {
    // start the spi for the touch screen and init the library
    spi3.begin(TOUCH_SCK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    touch_screen.begin(spi3);
    touch_screen.setRotation(display_orientation == TFT_ORIENTATION ? 0 : 1);

    // initiate display
    display.init();
    display.initDMA(true);
    display.setRotation(display_orientation == TFT_ORIENTATION ? 0 : 1);
    display.setAddrWindow(0, 0, display_width, display_height);
  }

  auto display_is_touched() -> bool override {
    return touch_screen.tirqTouched() && touch_screen.touched();
  }

  auto display_get_touch(uint16_t &x, uint16_t &y,
                         uint8_t &pressure) -> void override {
    touch_screen.readData(&x, &y, &pressure);
  }

  auto dma_write_bytes(uint8_t const *data, uint32_t len) -> void override {
    // note. TFT_eSPI requires non-const data in case bytes are swapped
    display.pushPixelsDMA(
        reinterpret_cast<uint16_t *>(const_cast<uint8_t *>(data)),
        len / sizeof(uint16_t));
  }

  auto dma_is_busy() -> bool override { return display.dmaBusy(); }

  auto dma_wait_for_completion() -> void override { return display.dmaWait(); }
};