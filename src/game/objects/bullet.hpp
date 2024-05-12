#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "fragment.hpp"
#include "game_object.hpp"

class bullet final : public game_object {
public:
  bullet() : game_object{bullet_cls} {
    col_bits = cb_enemy_bullet;
    col_mask = cb_hero;
    damage = 1;

    spr = sprites.allocate_instance();
    spr->obj = this;
    spr->img = sprite_imgs[1];
    spr->layer = 1;
    spr->flip = 0;
  }

  // returns true if object died
  auto update() -> bool override {
    if (game_object::update()) {
      return true;
    }
    if (x <= -sprite_width || x >= display_width || y <= -sprite_height ||
        y >= display_height) {
      return true;
    }
    return false;
  }

  void on_death_by_collision() override {
    fragment *frg = new (objects.allocate_instance()) fragment{};
    frg->die_at_ms = clk.ms + 250;
    frg->x = x;
    frg->y = y;
  }
};
