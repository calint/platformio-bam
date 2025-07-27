#pragma once
// platform dependent constants used by main, engine and game

// reviewed: 2023-12-11
// reviewed: 2024-05-01
// reviewed: 2024-05-22

// display dimensions depending on orientation
// using device natural orientation and requested orientation
static int constexpr display_width =
    display_orientation == TFT_ORIENTATION ? TFT_WIDTH : TFT_HEIGHT;
static int constexpr display_height =
    display_orientation == TFT_ORIENTATION ? TFT_HEIGHT : TFT_WIDTH;

// calibration of touch screen
static int16_t constexpr touch_screen_min_x = TOUCH_MIN_X;
static int16_t constexpr touch_screen_max_x = TOUCH_MAX_X;
static int16_t constexpr touch_screen_range_x =
    touch_screen_max_x - touch_screen_min_x;
static int16_t constexpr touch_screen_min_y = TOUCH_MIN_Y;
static int16_t constexpr touch_screen_max_y = TOUCH_MAX_Y;
static int16_t constexpr touch_screen_range_y =
    touch_screen_max_y - touch_screen_min_y;
