#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "bullet.hpp"
#include "fragment.hpp"
#include "upgrade.hpp"
// then other
#include "utils.hpp"

class hero final : public game_object {
    sprite* spr_left_ = nullptr;
    sprite* spr_right_ = nullptr;
    clk::time last_upgrade_deployed_ms_ = 0;
    static clk::time constexpr upgrade_deploy_interval_ms_ = 10000;

  public:
    hero() : game_object{cls_hero} {
        col_bits = cb_hero;
        col_mask = cb_enemy | cb_enemy_bullet;

        health = 10;

        spr = sprites.alloc();
        spr->obj = this;
        spr->img = sprite_imgs[0];
        spr->layer = 1; // put in top layer
        spr->flip = 0;

        spr_left_ = sprites.alloc();
        spr_left_->obj = this;
        spr_left_->img = sprite_imgs[0];
        spr_left_->layer = 1;
        spr_left_->flip = 0;

        spr_right_ = sprites.alloc();
        spr_right_->obj = this;
        spr_right_->img = sprite_imgs[0];
        spr_right_->layer = 1;
        spr_right_->flip = 0;

        last_upgrade_deployed_ms_ = clk.ms;

        state.hero_is_alive = true;
    }

    ~hero() override {
        // turn off and free sprites
        spr_left_->img = nullptr;
        sprites.free(spr_left_);
        spr_right_->img = nullptr;
        sprites.free(spr_right_);

        state.hero_is_alive = false;
    }

    auto update() -> bool override {
        if (!game_object::update()) {
            return false;
        }

        if (x >= display_width) {
            dx = -dx;
            x = display_width;
        } else if (x <= -sprite_width) {
            dx = -dx;
            x = -sprite_width;
        }

        if (clk.ms - last_upgrade_deployed_ms_ > upgrade_deploy_interval_ms_) {
            upgrade* upg = new (objects.alloc()) upgrade{};
            upg->x = x;
            upg->y = y;
            upg->dy = 30;
            upg->ddy = 20;
            last_upgrade_deployed_ms_ = clk.ms;
        }

        return true;
    }

    auto on_death_by_collision() -> void override {
        create_fragments(x, y, 16, 150, 2000);
    }

    auto pre_render() -> void override {
        game_object::pre_render();

        // set position of additional sprites
        spr_left_->scr_x = int16_t(spr->scr_x - sprite_width);
        spr_left_->scr_y = spr->scr_y;

        spr_right_->scr_x = int16_t(spr->scr_x + sprite_width);
        spr_right_->scr_y = spr->scr_y;
    }
};
