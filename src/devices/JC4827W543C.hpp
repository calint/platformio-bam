#pragma once

#include "JC4827W543.hpp"
#include <Touch_GT911.h>

/// @brief Implements touch screen for capacitive version
class JC4827W543C final : public JC4827W543 {
    Touch_GT911 touch_screen{
        TOUCH_SDA,
        TOUCH_SCK,
        TOUCH_IRQ,
        TOUCH_RST,
        display_orientation == TFT_ORIENTATION ? TFT_WIDTH : TFT_HEIGHT,
        display_orientation == TFT_ORIENTATION ? TFT_HEIGHT : TFT_WIDTH};

  protected:
    auto init_touch_screen() -> void override {
        touch_screen.begin();
        if (display_orientation == TFT_ORIENTATION) {
            touch_screen.setRotation(ROTATION_INVERTED);
        } else {
            touch_screen.setRotation(ROTATION_RIGHT);
        }
    }

  public:
    auto display_is_touched() -> bool override {
        touch_screen.read();
        // printf("touched: %u\n", touch_screen.isTouched);
        return touch_screen.isTouched;
    }

    auto display_touch_count() -> uint8_t override {
        //        printf("touches: %u\n", touch_screen.touches);
        return touch_screen.touches;
    }

    auto display_get_touch(touch touches[]) -> void override {
        //       printf("get touches: %u\n", touch_screen.touches);
        for (int i = 0; i < touch_screen.touches; i++) {
            touches[i].x = touch_screen.points[i].x;
            touches[i].y = touch_screen.points[i].y;
            touches[i].pressure = touch_screen.points[i].size;
            // printf("Touch %u: x = %u, y = %u, size = %u\n", i, touches[i].x,
            //      touches[i].y, touches[i].pressure);
        }
    }
};
