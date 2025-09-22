#pragma once
// setup initial game state, callbacks from engine, game logic
// solves circular references between 'state' and game objects

// reviewed: 2005-05-01
// reviewed: 2005-05-22

// device interface
#include "../device.hpp"
// the game state
#include "state.hpp"
// base class
#include "game_object.hpp"
// then the objects
#include "objects/ben.hpp"
#include "objects/bullet.hpp"
#include "objects/dummy.hpp"
#include "objects/hero.hpp"
#include "objects/ned.hpp"
#include "objects/ship1.hpp"
#include "objects/ship2.hpp"
#include "objects/ufo2.hpp"
// then other
#include "objects/utils.hpp"

// movement of tile map in pixels per second
static float tile_map_dx = 0;
static float tile_map_dy = 0;

// callback from 'setup()'
static auto application_init() -> void {
    printf("------------------- game object sizes --------------------\n");
    printf("          animator: %zu B\n", sizeof(animator));
    printf("       game_object: %zu B\n", sizeof(game_object));
    printf("            bullet: %zu B\n", sizeof(bullet));
    printf("             dummy: %zu B\n", sizeof(dummy));
    printf("          fragment: %zu B\n", sizeof(fragment));
    printf("              hero: %zu B\n", sizeof(hero));
    printf("             ship1: %zu B\n", sizeof(ship1));
    printf("             ship2: %zu B\n", sizeof(ship2));
    printf("           upgrade: %zu B\n", sizeof(upgrade));
    printf("    upgrade_picked: %zu B\n", sizeof(upgrade_picked));
    printf("              ufo2: %zu B\n", sizeof(ufo2));
    printf("               ben: %zu B\n", sizeof(ben));
    printf("               ned: %zu B\n", sizeof(ned));

    // assert that game object instances fit in object store slots
    static_assert(
        max_size_of_type<game_object, bullet, dummy, fragment, hero, ship1,
                         ship2, upgrade, upgrade_picked, ufo2, ben, ned>() <=
        object_instance_max_size_B);

    // scrolling vertically from bottom up
    tile_map_y = tile_map_height * tile_height - display_height;
    tile_map_dy = -16;

    ben* bn = new (objects.alloc()) ben{};
    bn->x = 0;
    bn->y = 80;

    ned* nd = new (objects.alloc()) ned{};
    nd->x = display_width - sprite_width;
    nd->y = 80;
    nd->moving_direction = -1;

    // create default hero
    hero* hro = new (objects.alloc()) hero{};
    hro->x = display_width / 2 - sprite_width / 2;
    hro->y = 30;
}

// callback when screen is touched, happens before 'render(...)'
static auto application_on_touch(device::touch const touches[],
                                 uint8_t const count) -> void {
    // keep track of when the previous bullet was fired
    static clk::time last_fire_ms = 0;

    // fire eight times a second
    if (clk.ms - last_fire_ms > 125) { // clear stats for this frame

        last_fire_ms = clk.ms;
        for (uint8_t i = 0; i < count; ++i) {
            if (object* mem = objects.alloc()) {
                bullet* blt = new (mem) bullet{};
                blt->x = display_x_for_touch(touches[i].x);
                blt->y = display_y_for_touch(touches[i].y);
                blt->dy = -200;
            }
        }
    }
}

//
// object waves related
//

// forward declaration of functions that start waves of objects
static auto wave_1() -> void;
static auto wave_2() -> void;
static auto wave_3() -> void;
static auto wave_4() -> void;
static auto wave_5() -> void;

// pointer to function that creates wave
using wave_func_ptr = auto (*)() -> void;

// eases placement of when waves should happen
static auto constexpr y_for_screen_percentage(float const screen_percentage)
    -> float {
    return float(display_height * screen_percentage / 100.0f);
}

struct wave_trigger {
    float since_last_wave_y = 0;
    wave_func_ptr func = nullptr;

    constexpr wave_trigger(float const y, wave_func_ptr const func_ptr)
        : since_last_wave_y{y}, func{func_ptr} {}
    // note: constructor needed for C++11 to compile

} static constexpr wave_triggers[] = {
    {y_for_screen_percentage(0), wave_5},
    {y_for_screen_percentage(50), wave_4},
    {y_for_screen_percentage(25), wave_1},
    {y_for_screen_percentage(50), wave_2},
    {y_for_screen_percentage(50), wave_3},
    {y_for_screen_percentage(100), wave_4},
    {y_for_screen_percentage(50), wave_3},
    {y_for_screen_percentage(50), wave_2},
    {y_for_screen_percentage(50), wave_1},
    {y_for_screen_percentage(50), wave_4},
};

// largest tile map y
static float constexpr wave_triggers_bottom_screen_y =
    tile_map_height * tile_height - display_height;

static int constexpr wave_triggers_len =
    sizeof(wave_triggers) / sizeof(wave_trigger);

static int wave_triggers_ix = 0;

// initiate to when the first wave is triggered
static float wave_triggers_next_y =
    wave_triggers_bottom_screen_y - wave_triggers[0].since_last_wave_y;

// callback after frame has been rendered and objects updated
// note: if objects are deleted see 'objects::update()'
static auto application_on_frame_completed() -> void {
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

    if (!state.hero_is_alive) {
        hero* hro = new (objects.alloc()) hero{};
        hro->x = random_float(0, display_width);
        hro->y = 30;
        hro->dx = random_float(-64, 64);
    }

    // trigger waves
    if (wave_triggers_ix < wave_triggers_len &&
        wave_triggers_next_y >= tile_map_y) {
        wave_triggers[wave_triggers_ix].func();
        ++wave_triggers_ix;
        if (wave_triggers_ix < wave_triggers_len) {
            wave_triggers_next_y -=
                wave_triggers[wave_triggers_ix].since_last_wave_y;
        }
    }
}

static auto wave_1() -> void {
    int constexpr count = display_width / (sprite_width * 3 / 2);
    int constexpr dx = display_width / count;
    float x = 0;
    float y = -sprite_height;
    for (int i = 0; i < count; ++i) {
        ship1* shp = new (objects.alloc()) ship1{};
        shp->x = x;
        shp->y = y;
        shp->dy = 50;
        x += dx;
        y -= sprite_width / 2;
    }
}

static auto wave_2() -> void {
    int constexpr count = display_width / (sprite_width * 3 / 2);
    int constexpr dx = display_width / count;
    float x = 0;
    float y = -sprite_height;
    for (int i = 0; i < count; ++i, x += dx) {
        ship1* shp = new (objects.alloc()) ship1{};
        shp->x = x;
        shp->y = y;
        shp->dy = 50;
    }
}

static auto wave_3() -> void {
    int constexpr count_y = 10;
    int constexpr count_x = 10;
    int constexpr dx = display_width / count_x;
    float y = -sprite_height;
    for (int j = 0; j < count_y; ++j, y -= 24) {
        float x = 0;
        for (int i = 0; i < count_x; ++i, x += dx) {
            ship1* shp = new (objects.alloc()) ship1{};
            shp->x = x;
            shp->y = y;
            shp->dy = 50;
        }
    }
}

static auto wave_4() -> void {
    ufo2* ufo = new (objects.alloc()) ufo2{};
    ufo->x = display_width / 2;
    ufo->y = -sprite_height;
    ufo->dy = 5;

    {
        ship2* shp = new (objects.alloc()) ship2{};
        shp->x = -sprite_width;
        shp->y = -sprite_height;
        shp->dy = 25;
        shp->dx = 12;
        shp->ddy = 20;
        shp->ddx = 10;
    }
    {
        ship2* shp = new (objects.alloc()) ship2{};
        shp->x = display_width;
        shp->y = -sprite_height;
        shp->dy = 25;
        shp->dx = -12;
        shp->ddy = 20;
        shp->ddx = -10;
    }
}

static auto wave_5() -> void {
    float y = -float(sprite_height);
    float constexpr dx = display_width / 12;
    for (int j = 0; j < 12; ++j, y -= 16) {
        float x = 0;
        for (int i = 0; i < 19; ++i, x += dx) {
            ship1* shp = new (objects.alloc()) ship1{};
            shp->x = x;
            shp->y = y;
            shp->dy = 30;
        }
    }
}