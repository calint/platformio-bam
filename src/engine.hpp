#pragma once
// platform-independent game engine code

// reviewed: 2023-12-11
// reviewed: 2024-05-01
// reviewed: 2024-05-22

// calibration of touch screen (defined in `platformio.ini`)
static int16_t constexpr touch_screen_min_x = TOUCH_MIN_X;
static int16_t constexpr touch_screen_max_x = TOUCH_MAX_X;
static int16_t constexpr touch_screen_range_x =
    touch_screen_max_x - touch_screen_min_x;
static int16_t constexpr touch_screen_min_y = TOUCH_MIN_Y;
static int16_t constexpr touch_screen_max_y = TOUCH_MAX_Y;
static int16_t constexpr touch_screen_range_y =
    touch_screen_max_y - touch_screen_min_y;
// --

#include "o1store.hpp"

#include <cstdint>
#include <limits>

// palette used when rendering tile images
// converts uint8_t to uint16_t rgb 565 (red being the highest bits)
// note: lower and higher byte swapped
static uint16_t constexpr palette_tiles[256]{
#include "application/resources/palette_tiles.hpp"
};

// palette used when rendering sprites
static uint16_t constexpr palette_sprites[256]{
#include "application/resources/palette_sprites.hpp"
};

// images used by tile map
static uint8_t constexpr tile_imgs[tile_img_count][tile_width * tile_height]{
#include "application/resources/tile_imgs.hpp"
};

// initial tile map
static tile_img_ix tile_map[tile_map_height][tile_map_width]{
#include "application/resources/tile_map.hpp"
};

// initial tile map cell flags
static uint8_t tile_map_flags[tile_map_height][tile_map_width]{
#include "application/resources/tile_map_flags.hpp"
};

// render tile map starting at top left pixel position
static float tile_map_x = 0;
static float tile_map_y = 0;

// images used by sprites
static uint8_t constexpr sprite_imgs[sprite_img_count]
                                    [sprite_width * sprite_height]{
#include "application/resources/sprite_imgs.hpp"
                                    };

using sprite_img = uint8_t const*;

// the reserved 'sprite_ix' in 'collision_map' representing 'no sprite pixel'
static sprite_ix constexpr sprite_ix_reserved =
    std::numeric_limits<sprite_ix>::max();

// forward declaration of type
class object;

class sprite final {
  public:
    sprite** alloc_ptr = nullptr;
    object* obj = nullptr;
    sprite_img img = nullptr;
    int16_t scr_x = 0;
    int16_t scr_y = 0;
    uint8_t layer = 0;
    // note: lower 'layer' number is rendered first
    //       number of layers specified by 'sprite_layer_count'
    uint8_t flip = 0; // bits: horiz: 0b01, vert: 0b10
};

using sprites_store = o1store<sprite, sprite_count, 1>;

static sprites_store sprites{};

class object {
  public:
    object** alloc_ptr;
    // note: no default value since it would overwrite the 'o1store' assigned
    //       value at 'alloc()'

    object* col_with = nullptr;
    collision_bits col_bits = 0;
    collision_bits col_mask = 0;
    // note: used to declare interest in collisions with objects whose
    //       'col_bits' bitwise AND with this 'col_mask' is not 0

    virtual ~object() = default;
    // note: 'delete obj' is not allowed since memory is managed by 'o1store'

    // returns false if object has died
    // note: regarding classes overriding 'update(...)'
    //       after 'update(...)' 'col_with' should be 'nullptr'
    virtual auto update() -> bool { return true; }

    // called before rendering the sprites
    virtual auto pre_render() -> void {}
};

using object_store =
    o1store<object, object_count, 2, object_instance_max_size_B>;

class objects : public object_store {
  public:
    auto update() -> void {
        object const* const* end = allocated_list_end();
        // note: important to get the 'end' outside the loop because objects may
        //       allocate new objects in the loop and that would change the
        //       'end'
        for (object** it = allocated_list(); it < end; ++it) {
            object* obj = *it;
            if (!obj->update()) {
                obj->~object();
                free(obj);
            }
        }
    }

    auto pre_render() -> void {
        object const* const* end = allocated_list_end();
        // note: important to get the 'end' outside the loop because objects may
        //       allocate new objects in the loop and that would change the
        //       'end'
        for (object** it = allocated_list(); it < end; ++it) {
            object* obj = *it;
            obj->pre_render();
        }
    }
} static objects{};

// helper class managing current frame time, dt, frames per second calculation
class clk {
  public:
    using time = uint32_t;

  private:
    int interval_ms_ = 0;
    int frames_rendered_since_last_update_ = 0;
    time last_fps_update_ms_ = 0;
    time prv_ms_ = 0;
    int locked_dt_ms_ = 0;

  public:
    // current time since boot in milliseconds
    time ms = 0;

    // frame delta time in seconds
    float dt = 0;

    // current frames per second calculated at interval specified at 'init'
    int fps = 0;

    // called at setup with current time, frames per seconds calculation
    // interval and optional fixed frame delta time
    auto init(time const time_ms, int const interval_of_fps_calculation_ms,
              int const locked_dt_ms) -> void {
        interval_ms_ = interval_of_fps_calculation_ms;
        if (locked_dt_ms) {
            locked_dt_ms_ = locked_dt_ms;
            dt = 0.001f * float(locked_dt_ms);
        } else {
            prv_ms_ = ms = time_ms;
        }
        last_fps_update_ms_ = time_ms;
    }

    // called before every frame to update state
    // returns true if new frames per second calculation was done
    auto on_frame(time const time_ms) -> bool {
        if (locked_dt_ms_) {
            ms += time(locked_dt_ms_);
        } else {
            ms = time_ms;
            dt = 0.001f * float(ms - prv_ms_);
            if (dt > 0.1f) {
                dt = 0.1f; // the rollover and dt cap
            }
            prv_ms_ = ms;
        }
        frames_rendered_since_last_update_++;
        time const dt_ms = time_ms - last_fps_update_ms_;
        if (dt_ms >= interval_ms_) {
            fps = frames_rendered_since_last_update_ * 1000 / dt_ms;
            frames_rendered_since_last_update_ = 0;
            last_fps_update_ms_ = time_ms;
            return interval_ms_ != 0;
        }
        return false;
    }
} static clk{};

// callback from 'main.cpp'
static auto engine_init() -> void {
    // set random seed for deterministic behavior
    srand(random_seed);
}

// forward declaration of platform specific function
static auto render(int x, int y) -> void;

// forward declaration of user provided callback
static auto application_on_frame_completed() -> void;

// callback from 'main.cpp'
// render and update the state of the engine
static auto engine_loop() -> void {
    // prepare objects for render
    objects.pre_render();

    // render tiles, sprites and collision map
    render(int(tile_map_x), int(tile_map_y));

    // call 'update()' on allocated objects
    objects.update();

    // deallocate the objects freed during 'objects.update()'
    objects.apply_free();

    // deallocate the sprites freed during 'objects.update()'
    sprites.apply_free();

    // application logic hook
    application_on_frame_completed();
}

// used for static assert of object sizes and config
// 'object_instance_max_size_B'
template <typename T> static auto constexpr max_size_of_type() -> int {
    return sizeof(T);
}

template <typename T, typename U, typename... Args>
static auto constexpr max_size_of_type() -> int {
    return sizeof(T) > max_size_of_type<U, Args...>()
               ? sizeof(T)
               : max_size_of_type<U, Args...>();
}

// returns a random float
static auto random_float(float const min, float const max) -> float {
    float constexpr rand_max_inv = 1.0f / float(RAND_MAX);
    return (max - min) * float(rand()) * rand_max_inv + min;
}

// returns x on display for raw touch value x
static auto display_x_for_touch(int16_t const x) -> float {
    static float constexpr fact_x = float(display_width) / touch_screen_range_x;
    return float(x - touch_screen_min_x) * fact_x;
}

// returns y on display for raw touch value y
static auto display_y_for_touch(int16_t const y) -> float {
    static float constexpr fact_y =
        float(display_height) / touch_screen_range_y;
    return float(y - touch_screen_min_y) * fact_y;
}
