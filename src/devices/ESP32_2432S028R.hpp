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
    TouchPoint touch_point{};

  public:
    auto init() -> void override {
        // initiate display
        display.init();
        display.initDMA(true);
        display.setRotation(display_orientation == TFT_ORIENTATION ? 0 : 1);
        display.setAddrWindow(0, 0, display_width, display_height);

        touch_screen.begin();

        spi3.begin(SD_SCK, SD_MISO, SD_MOSI);
        init_sd_spiffs(spi3, SD_CS, 80000000);
    }

    auto display_is_touched() -> bool override {
        touch_point = touch_screen.getTouch();
        return touch_point.zRaw != 0;
    }

    auto display_touch_count() -> uint8_t override { return 1; }

    auto display_get_touch(touch touches[]) -> void override {
        // flip x and y for display orientation
        touches[0].x = 4096 - touch_point.yRaw;
        touches[0].y = touch_point.xRaw;
        touches[0].pressure = touch_point.zRaw;
    }

    auto dma_write_bytes(uint8_t const* data, uint32_t const len)
        -> void override {
        // note. TFT_eSPI requires non-const data in case bytes are swapped
        // note. pushPixelsDMA(...) waits for previous transaction to complete
        display.pushPixelsDMA(
            reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(data)),
            len / sizeof(uint16_t));
    }

    auto dma_is_busy() -> bool override { return display.dmaBusy(); }

    auto dma_wait_for_completion() -> void override {
        return display.dmaWait();
    }
};