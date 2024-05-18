#pragma once
// display: ILI9341
//     bus: SPI
//
// implements device using bb_spi_lcd

#include "../device.hpp"
#include "hal/spi_types.h"
#include <SPI.h>
#include <bb_spi_lcd.h>

class ESP32_2432S028R final : public device {
  SPIClass vspi{VSPI};
  BB_SPI_LCD display{};
  TOUCHINFO touch_info{};

public:
  auto init() -> void override {
    // initiate display
    display.begin(DISPLAY_CYD);
    display.rtInit();
    display.setRotation(display_orientation == TFT_ORIENTATION ? 0 : 1);
    display.setAddrWindow(0, 0, display_width, display_height);

    vspi.begin(SD_SCK, SD_MISO, SD_MOSI);

    init_sd_spiffs(vspi, SD_CS, 80000000);
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
};