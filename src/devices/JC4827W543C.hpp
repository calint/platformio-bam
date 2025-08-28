#pragma once

#include "JC4827W543.hpp"
#include <Touch_GT911.h>

/// @brief Implements touch screen for capacitive version
class JC4827W543C final : public JC4827W543 {
    static int constexpr touch_irq = 3;
    static int constexpr touch_sda = 8;
    static int constexpr touch_sck = 4;
    static int constexpr touch_rst = 38;

    Touch_GT911 touch_screen{touch_sda,
                             touch_sck,
                             touch_irq,
                             touch_rst,
                             display_orientation == 1 ? 480 : 272,
                             display_orientation == 1 ? 272 : 480};

  protected:
    auto init_touch_screen() -> void override {

        // note: cold boot sometimes does not reset the i2c device
        //       reset using button sometimes fails
        //       the reset procedure below fixes that
        //       see: https://www.crystalfontz.com/controllers/GOODIX/GT911/
        //            page 10

        // GT911 hardware reset sequence
        pinMode(touch_rst, OUTPUT);
        pinMode(touch_irq, OUTPUT);

        // Step 1: Pull RST low
        digitalWrite(touch_rst, LOW);
        digitalWrite(touch_irq, LOW); // INT low sets I2C address
        delay(10);

        // Step 2: Release RST while keeping INT low
        digitalWrite(touch_rst, HIGH);
        delay(5);

        // Step 3: Release INT pin
        digitalWrite(touch_irq, HIGH);
        delay(50);

        // Step 4: Configure INT as input for interrupt
        pinMode(touch_irq, INPUT);

        touch_screen.begin();

        if (display_orientation == 1) {
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
        for (int i = 0; i < touch_screen.touches; ++i) {
            touches[i].x = touch_screen.points[i].x;
            touches[i].y = touch_screen.points[i].y;
            touches[i].pressure = touch_screen.points[i].size;
            // printf("Touch %u: x = %u, y = %u, size = %u\n", i, touches[i].x,
            //      touches[i].y, touches[i].pressure);
        }
    }
};
