#pragma once
// platform-independent game engine code

// reviewed: 2023-12-11
// reviewed: 2024-05-01

#include "o1store.hpp"

#include <cstdint>
#include <limits>

// palette used when rendering tiles
// converts uint8_t to uint16_t rgb 565 (red being the highest bits)
// note. lower and higher byte swapped
static constexpr uint16_t palette_tiles[256]{
#include "game/resources/palette_tiles.hpp"
};

// palette used when rendering sprites
static constexpr uint16_t palette_sprites[256]{
#include "game/resources/palette_sprites.hpp"
};

// images used by tile map
static constexpr uint8_t tiles[tiles_count][tile_width * tile_height]{
#include "game/resources/tiles.hpp"
};

// initial tile map
static tile_ix tile_map[tile_map_height][tile_map_width]{
#include "game/resources/tile_map.hpp"
};

// tile map controls
static float tile_map_x = 0;
static float tile_map_dx = 0;
static float tile_map_y = 0;
static float tile_map_dy = 0;

// images used by sprites
static constexpr uint8_t sprite_imgs[sprite_imgs_count]
                                    [sprite_width * sprite_height]{
#include "game/resources/sprite_imgs.hpp"
                                    };

// the reserved 'sprite_ix' in 'collision_map' representing 'no sprite pixel'
static constexpr sprite_ix sprite_ix_reserved =
    std::numeric_limits<sprite_ix>::max();

// forward declaration of type
class object;

class sprite {
public:
  sprite **alloc_ptr = nullptr;
  object *obj = nullptr;
  uint8_t const *img = nullptr;
  int16_t scr_x = 0;
  int16_t scr_y = 0;
  uint8_t layer = 0;
  // note. lower 'layer' number is rendered first
  //       number of layers specified by 'sprites_layers'
  uint8_t flip = 0; // bits: horiz: 0b01, vert: 0b10
};

using sprites_store = o1store<sprite, sprites_count, 1>;

static sprites_store sprites{};

class object {
public:
  object **alloc_ptr;
  // note. no default value since it would overwrite the 'o1store' assigned
  //       value at 'allocate_instance()'

  object *col_with = nullptr;
  collision_bits col_bits = 0;
  collision_bits col_mask = 0;
  // note. used to declare interest in collisions with objects whose
  //       'col_bits' bitwise AND with this 'col_mask' is not 0

  virtual ~object() {}
  // note. 'delete obj' is not allowed since memory is managed by 'o1store'

  // returns true if object has died
  // note. regarding classes overriding 'update(...)'
  //       after 'update(...)' 'col_with' should be 'nullptr'
  virtual auto update() -> bool { return false; }

  // called before rendering the sprites
  virtual void pre_render() {}
};

using object_store =
    o1store<object, objects_count, 2, object_instance_max_size_B>;

class objects : public object_store {
public:
  void update() {
    object **const end = allocated_list_end();
    // note. important to get the 'end' outside the loop because objects may
    //       allocate new objects in the loop and that would change the 'end'
    for (object **it = allocated_list(); it < end; it++) {
      object *obj = *it;
      if (obj->update()) {
        obj->~object();
        free_instance(obj);
      }
    }
  }

  void pre_render() {
    object **const end = allocated_list_end();
    // note. important to get the 'end' outside the loop because objects may
    //       allocate new objects in the loop and that would change the 'end'
    for (object **it = allocated_list(); it < end; it++) {
      object *obj = *it;
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
  void init(const time time_ms, const int interval_of_fps_calculation_ms,
            const int locked_dt_ms) {
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
  auto on_frame(const time time_ms) -> bool {
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
    const time dt_ms = time_ms - last_fps_update_ms_;
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
static void engine_init() {
  // set random seed for deterministic behavior
  srand(random_seed);
}

// forward declaration of platform specific function
static void render(int x, int y);

// forward declaration of user provided callback
static void main_on_frame_completed();

// callback from 'main.cpp'
// render and update the state of the engine
static void engine_loop() {
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

  // game logic hook
  main_on_frame_completed();
}

// used for static assert of object sizes and config
// 'object_instance_max_size_B'
template <typename T> static constexpr int max_size_of_type() {
  return sizeof(T);
}

template <typename T, typename U, typename... Args>
static constexpr int max_size_of_type() {
  return sizeof(T) > max_size_of_type<U, Args...>()
             ? sizeof(T)
             : max_size_of_type<U, Args...>();
}

// returns a random float
static float random_float(const float min, const float max) {
  constexpr float rand_max_inv = 1.0f / float(RAND_MAX);
  return (max - min) * float(rand()) * rand_max_inv + min;
}

// returns x on display for raw touch value x
static float display_x_for_touch(const int16_t x) {
  static constexpr float fact_x = float(display_width) / touch_screen_range_x;
  return float(x - touch_screen_min_x) * fact_x;
}

// returns y on display for raw touch value y
static float display_y_for_touch(const int16_t y) {
  static constexpr float fact_y = float(display_height) / touch_screen_range_y;
  return float(y - touch_screen_min_y) * fact_y;
}
