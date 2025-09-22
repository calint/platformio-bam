#pragma once
// first include engine
#include "../engine.hpp"

// implements common behavior of all game objects
class game_object : public object {
  public:
    sprite* spr = nullptr;

    float ddx = 0;
    float dx = 0;
    float x = 0;
    float ddy = 0;
    float dy = 0;
    float y = 0;

    int16_t health = 0;

    // damage inflicted on other object at collision
    int16_t damage = 0;

    // run time information about the class of this object
    object_class cls;

    explicit game_object(object_class c) : cls{c} {}
    // note: after constructor 'spr' must be in valid state.

    ~game_object() override {
        // turn off and free sprite
        spr->img = nullptr;
        sprites.free(spr);
    }

    // returns false if object has died
    // note: regarding classes overriding 'update(...)'
    // after 'update(...)' 'col_with' should be 'nullptr'
    auto update() -> bool override {
        // note: object::update() omitted since it simply returns 'true'

        if (col_with) {
            if (!on_collision(static_cast<game_object*>(col_with))) {
                return false;
            }
            col_with = nullptr;
        }

        dx += ddx * clk.dt;
        x += dx * clk.dt;
        dy += ddy * clk.dt;
        y += dy * clk.dt;

        return true;
    }

    // called before rendering the sprites
    auto pre_render() -> void override {
        spr->scr_x = int16_t(x);
        spr->scr_y = int16_t(y);
    }

    // called from 'update' if object is in collision
    // returns false if object has died
    virtual auto on_collision(game_object* obj) -> bool {
        health = int16_t(health - obj->damage);
        if (health <= 0) {
            on_death_by_collision();
            return false;
        }
        return true;
    }

    // called from 'on_collision' if object has died due to collision
    virtual auto on_death_by_collision() -> void {}
};
