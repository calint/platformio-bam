#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "ship2.hpp"
// then other
#include "utils.hpp"

class ufo2 final : public game_object {
    sprites_2x2 sprs_;

  public:
    ufo2()
        : game_object{cls_ufo2}, sprs_{this, 10, 1, sprites_2x2::mirror::none} {
        col_bits = cb_hero;
        col_mask = cb_enemy | cb_enemy_bullet;

        health = 100;
    }

    auto pre_render() -> void override { sprs_.pre_render(this); }

    auto update() -> bool override {
        if (!game_object::update()) {
            return false;
        }

        if (y > (display_height + sprite_height)) {
            return false;
        }

        return true;
    }

    auto on_collision(game_object* obj) -> bool override {
        ship2* shp = new (objects.alloc()) ship2{};
        shp->x = obj->x;
        shp->y = obj->y - sprite_height;
        shp->dx = random_float(-100, 100);
        shp->ddx = -shp->dx * 0.5f;
        shp->dy = -100;
        shp->ddy = 100;

        return game_object::on_collision(obj);
    }

    auto on_death_by_collision() -> void override {
        create_fragments(x, y, 32, 150, 2000);
    }
};
