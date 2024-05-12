#pragma once

#include "../device.hpp"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

class ESP32_2432S028R final : public device {
  TFT_eSPI display{};
  // init touch screen
  SPIClass hspi{HSPI}; // note. VSPI is used by the display
  XPT2046_Touchscreen touch_screen{TOUCH_CS, TOUCH_IRQ};

public:
  void init() override {
    // start the spi for the touch screen and init the library
    hspi.begin(TOUCH_SCK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    touch_screen.begin(hspi);
    touch_screen.setRotation(display_orientation);

    // initiate display
    display.init();
    display.setRotation(display_orientation);
    display.setAddrWindow(0, 0, display_width, display_height);
    display.initDMA(true);
  }

  auto is_display_touched() -> bool override {
    return touch_screen.tirqTouched() && touch_screen.touched();
  }

  void get_display_touch(int16_t &x, int16_t &y, int16_t &z) override {
    const TS_Point pt = touch_screen.getPoint();
    x = pt.x;
    y = pt.y;
    z = pt.z;
  }

  virtual auto asyncDMAIsBusy() -> bool override { return display.dmaBusy(); }

  void asyncDMAWaitForCompletion() override { display.dmaWait(); }

  void asyncDMAWriteBytes(uint8_t *data, uint32_t len) override {
    display.pushPixelsDMA(reinterpret_cast<uint16_t *>(data), len >> 1);
  }
};