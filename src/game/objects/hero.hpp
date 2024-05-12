#pragma once
// first include engine
#include "../../engine.hpp"
// then game state
#include "../game_state.hpp"
// then objects
#include "bullet.hpp"
#include "fragment.hpp"
#include "game_object.hpp"
#include "upgrade.hpp"
// then other
#include "utils.hpp"

class hero final : public game_object {
  sprite *spr_left = nullptr;
  sprite *spr_right = nullptr;
  clk::time last_upgrade_deployed_ms = 0;
  static constexpr clk::time upgrade_deploy_interval_ms = 10000;

public:
  hero() : game_object{hero_cls} {
    col_bits = cb_hero;
    col_mask = cb_enemy | cb_enemy_bullet;

    health = 10;

    spr = sprites.allocate_instance();
    spr->obj = this;
    spr->img = sprite_imgs[0];
    spr->layer = 1; // put in top layer
    spr->flip = 0;

    spr_left = sprites.allocate_instance();
    spr_left->obj = this;
    spr_left->img = sprite_imgs[0];
    spr_left->layer = 1;
    spr_left->flip = 0;

    spr_right = sprites.allocate_instance();
    spr_right->obj = this;
    spr_right->img = sprite_imgs[0];
    spr_right->layer = 1;
    spr_right->flip = 0;

    last_upgrade_deployed_ms = clk.ms;

    game_state.hero_is_alive = true;
  }

  ~hero() override {
    // turn off and free sprites
    spr_left->img = nullptr;
    sprites.free_instance(spr_left);
    spr_right->img = nullptr;
    sprites.free_instance(spr_right);

    game_state.hero_is_alive = false;
  }

  // returns true if object died
  auto update() -> bool override {
    if (game_object::update()) {
      return true;
    }

    if (x >= display_width) {
      dx = -dx;
      x = display_width;
    } else if (x <= -sprite_width) {
      dx = -dx;
      x = -sprite_width;
    }

    if (clk.ms - last_upgrade_deployed_ms > upgrade_deploy_interval_ms) {
      upgrade *upg = new (objects.allocate_instance()) upgrade{};
      upg->x = x;
      upg->y = y;
      upg->dy = 30;
      upg->ddy = 20;
      last_upgrade_deployed_ms = clk.ms;
    }

    return false;
  }

  void on_death_by_collision() override {
    create_fragments(x, y, 16, 150, 2000);
  }

  void pre_render() override {
    game_object::pre_render();

    // set position of additional sprites
    spr_left->scr_x = int16_t(spr->scr_x - sprite_width);
    spr_left->scr_y = spr->scr_y;

    spr_right->scr_x = int16_t(spr->scr_x + sprite_width);
    spr_right->scr_y = spr->scr_y;
  }
};
