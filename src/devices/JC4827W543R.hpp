#pragma once

#include "JC4827W543.hpp"
#include <XPT2046_Touchscreen.h>
#include <driver/spi_master.h>

/// @brief Implements touch screen for resistive version
class JC4827W543R final : public JC4827W543 {
    static int constexpr touch_cs = 38;

    XPT2046_Touchscreen touch_screen{touch_cs};
    uint16_t touch_screen_x = 0;
    uint16_t touch_screen_y = 0;
    uint8_t touch_screen_pressure = 0;

  protected:
    auto init_touch_screen() -> void override {
        touch_screen.begin(spi2);
        touch_screen.setRotation(display_orientation == 1 ? 0 : 1);
    }

  public:
    auto display_is_touched() -> bool override {
        touch_screen.readData(&touch_screen_x, &touch_screen_y,
                              &touch_screen_pressure);
        return touch_screen_pressure != 0;
    }

    auto display_touch_count() -> uint8_t override { return 1; }

    auto display_get_touch(touch touches[]) -> void override {
        touches[0].x = touch_screen_x;
        touches[0].y = touch_screen_y;
        touches[0].pressure = touch_screen_pressure;
    }
};
