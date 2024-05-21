#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "game_object.hpp"
//
#include "../animator.hpp"

static constexpr animator::frame ship2_animation[]{
    {&sprite_imgs[64][0], 500, 0, 0},
    {&sprite_imgs[65][0], 500, 0, 0},
};

class ship2 final : public game_object {
  animator animator_{};

public:
  ship2() : game_object{ship2_cls} {
    col_bits = cb_hero;
    col_mask = cb_enemy_bullet;

    animator_.init(ship2_animation,
                   sizeof(ship2_animation) / sizeof(animator::frame), true);

    spr = sprites.alloc();
    spr->obj = this;
    spr->img = animator_.sprite_img();
    spr->layer = 1;
    spr->flip = 0;
  }

  // returns true if object died
  auto update() -> bool override {
    if (game_object::update()) {
      return true;
    }

    if (dy > 0) {
      spr->flip = 2; // vertical
    }

    if (y > display_height) {
      return true;
    }

    if (animator_.update()) {
      spr->img = animator_.sprite_img();
    }

    return false;
  }

  auto on_death_by_collision() -> void override {
    upgrade *upg = new (objects.alloc()) upgrade{};
    upg->x = x;
    upg->y = y;
    upg->dy = 30;
    upg->ddy = 20;
  }
};
