#pragma once
// setup initial game state, callbacks from engine, game logic
// solves circular references between 'game_state' and game objects

// reviewed: 2005-05-01

// then the game state
#include "game_state.hpp"
// then the objects
#include "objects/bullet.hpp"
#include "objects/dummy.hpp"
#include "objects/hero.hpp"
#include "objects/ship1.hpp"
#include "objects/ship2.hpp"
#include "objects/ufo2.hpp"
// then other
#include "objects/utils.hpp"

// callback from 'setup()'
static void main_init() {
  // output size of game object classes
  printf("------------------- game object sizes --------------------\n");
  printf("       game_object: %zu B\n", sizeof(game_object));
  printf("            bullet: %zu B\n", sizeof(bullet));
  printf("             dummy: %zu B\n", sizeof(dummy));
  printf("          fragment: %zu B\n", sizeof(fragment));
  printf("              hero: %zu B\n", sizeof(hero));
  printf("             ship1: %zu B\n", sizeof(ship1));
  printf("             ship2: %zu B\n", sizeof(ship2));
  printf("    upgrade_picked: %zu B\n", sizeof(upgrade_picked));
  printf("           upgrade: %zu B\n", sizeof(upgrade));
  printf("              ufo2: %zu B\n", sizeof(ufo2));

  // assert that game object instances fit in object store slots
  static_assert(
      max_size_of_type<game_object, bullet, dummy, fragment, hero, ship1, ship2,
                       upgrade_picked, upgrade, ufo2>() <=
      object_instance_max_size_B);

  // scrolling vertically from bottom up
  tile_map_y = tile_map_height * tile_height - display_height;
  tile_map_dy = -16;

  // create default hero
  hero *hro = new (objects.allocate_instance()) hero{};
  hro->x = display_width / 2 - sprite_width / 2;
  hro->y = 30;
}

// callback when screen is touched, happens before 'render(...)'
static void main_on_touch(const int16_t x, const int16_t y, const int16_t z) {
  // keep track of when the previous bullet was fired
  static clk::time last_fire_ms = 0;

  // fire eight times a second
  if (clk.ms - last_fire_ms > 125) {
    last_fire_ms = clk.ms;
    if (object *mem = objects.allocate_instance()) {
      bullet *blt = new (mem) bullet{};
      blt->x = display_x_for_touch(x);
      blt->y = display_y_for_touch(y);
      blt->dy = -200;
    }
  }
}

//
// object waves related
//

// forward declaration of functions that start waves of objects
static void main_wave_1();
static void main_wave_2();
static void main_wave_3();
static void main_wave_4();
static void main_wave_5();

// pointer to function that creates wave
using wave_func_ptr = void (*)();

// eases placement of when waves should happen
static constexpr float y_for_screen_percentage(const float screen_percentage) {
  return float(display_height * screen_percentage / 100.0f);
}

struct wave_trigger {
  float since_last_wave_y = 0;
  wave_func_ptr func = nullptr;

  constexpr wave_trigger(float y_, wave_func_ptr func_)
      : since_last_wave_y{y_}, func{func_} {}
  // note. constructor needed for C++11 to compile

} static constexpr wave_triggers[] = {
    {y_for_screen_percentage(0), main_wave_5},
    {y_for_screen_percentage(50), main_wave_4},
    {y_for_screen_percentage(25), main_wave_1},
    {y_for_screen_percentage(50), main_wave_2},
    {y_for_screen_percentage(50), main_wave_3},
    {y_for_screen_percentage(100), main_wave_4},
    {y_for_screen_percentage(50), main_wave_3},
    {y_for_screen_percentage(50), main_wave_2},
    {y_for_screen_percentage(50), main_wave_1},
    {y_for_screen_percentage(50), main_wave_4},
};

// largest tile map y
static constexpr float wave_triggers_bottom_screen_y =
    tile_map_height * tile_height - display_height;

static constexpr int wave_triggers_len =
    sizeof(wave_triggers) / sizeof(wave_trigger);

static int wave_triggers_ix = 0;

// initiate to when the first wave is triggered
static float wave_triggers_next_y =
    wave_triggers_bottom_screen_y - wave_triggers[0].since_last_wave_y;

// callback after frame has been rendered and objects updated
// note. if objects are deleted see objects::update()
static void main_on_frame_completed() {
  // update x position in pixels in the tile map
  tile_map_x += tile_map_dx * clk.dt;
  if (tile_map_x < 0) {
    tile_map_x = 0;
    tile_map_dx = -tile_map_dx;
  } else if (tile_map_x > (tile_map_width * tile_width - display_width)) {
    tile_map_x = tile_map_width * tile_width - display_width;
    tile_map_dx = -tile_map_dx;
  }
  // update y position in pixels in the tile map
  tile_map_y += tile_map_dy * clk.dt;
  if (tile_map_y < 0) {
    tile_map_y = 0;
    tile_map_dy = -tile_map_dy;
  } else if (tile_map_y > (tile_map_height * tile_height - display_height)) {
    tile_map_y = tile_map_height * tile_height - display_height;
    tile_map_dy = -tile_map_dy;
    // set y of first wave trigger
    wave_triggers_ix = 0;
    wave_triggers_next_y =
        wave_triggers_bottom_screen_y - wave_triggers[0].since_last_wave_y;
  }

  if (!game_state.hero_is_alive) {
    hero *hro = new (objects.allocate_instance()) hero{};
    hro->x = random_float(0, display_width);
    hro->y = 30;
    hro->dx = random_float(-64, 64);
  }

  // trigger waves
  if (wave_triggers_ix < wave_triggers_len &&
      wave_triggers_next_y >= tile_map_y) {
    wave_triggers[wave_triggers_ix].func();
    wave_triggers_ix++;
    if (wave_triggers_ix < wave_triggers_len) {
      wave_triggers_next_y -= wave_triggers[wave_triggers_ix].since_last_wave_y;
    }
  }
}

static void main_wave_1() {
  constexpr int count = display_width / (sprite_width * 3 / 2);
  constexpr int dx = display_width / count;
  float x = 0;
  float y = -sprite_height;
  for (int i = 0; i < count; i++) {
    ship1 *shp = new (objects.allocate_instance()) ship1{};
    shp->x = x;
    shp->y = y;
    shp->dy = 50;
    x += dx;
    y -= sprite_width / 2;
  }
}

static void main_wave_2() {
  constexpr int count = display_width / (sprite_width * 3 / 2);
  constexpr int dx = display_width / count;
  float x = 0;
  float y = -sprite_height;
  for (int i = 0; i < count; i++, x += dx) {
    ship1 *shp = new (objects.allocate_instance()) ship1{};
    shp->x = x;
    shp->y = y;
    shp->dy = 50;
  }
}

static void main_wave_3() {
  constexpr int count_y = 10;
  constexpr int count_x = 10;
  constexpr int dx = display_width / count_x;
  float y = -sprite_height;
  for (int j = 0; j < count_y; j++, y -= 24) {
    float x = 0;
    for (int i = 0; i < count_x; i++, x += dx) {
      ship1 *shp = new (objects.allocate_instance()) ship1{};
      shp->x = x;
      shp->y = y;
      shp->dy = 50;
    }
  }
}

static void main_wave_4() {
  ufo2 *ufo = new (objects.allocate_instance()) ufo2{};
  ufo->x = display_width / 2;
  ufo->y = -sprite_height;
  ufo->dy = 5;

  {
    ship2 *shp = new (objects.allocate_instance()) ship2{};
    shp->x = -sprite_width;
    shp->y = -sprite_height;
    shp->dy = 25;
    shp->dx = 12;
    shp->ddy = 20;
    shp->ddx = 10;
  }
  {
    ship2 *shp = new (objects.allocate_instance()) ship2{};
    shp->x = display_width;
    shp->y = -sprite_height;
    shp->dy = 25;
    shp->dx = -12;
    shp->ddy = 20;
    shp->ddx = -10;
  }
}

static void main_wave_5() {
  float y = -float(sprite_height);
  constexpr float dx = display_width / 12;
  for (int j = 0; j < 12; j++, y -= 10) {
    float x = 0;
    for (int i = 0; i < 19; i++, x += dx) {
      ship1 *shp = new (objects.allocate_instance()) ship1{};
      shp->x = x;
      shp->y = y;
      shp->dy = 30;
    }
  }
}