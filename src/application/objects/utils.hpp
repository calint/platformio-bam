#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "fragment.hpp"

class sprites_2x2 final {
    // three additional sprites
    sprite* sprs[3];

  public:
    enum class mirror : uint8_t { none, all, horizontal, vertical };

    sprites_2x2(game_object* obj, int const top_left_index_in_16_sprites_row,
                uint8_t const layer, mirror const mirror = mirror::none) {
        obj->spr = sprites.alloc();
        obj->spr->obj = obj;
        obj->spr->layer = layer;
        obj->spr->flip = 0;
        // additional 3 sprites
        for (sprite*& spr : sprs) {
            spr = sprites.alloc();
            spr->obj = obj;
            spr->layer = layer;
            spr->flip = 0;
        }
        obj->spr->img = sprite_imgs[top_left_index_in_16_sprites_row];
        switch (mirror) {
        case mirror::none:
            sprs[0]->img = sprite_imgs[top_left_index_in_16_sprites_row + 1];
            sprs[1]->img = sprite_imgs[top_left_index_in_16_sprites_row + 16];
            sprs[2]->img =
                sprite_imgs[top_left_index_in_16_sprites_row + 16 + 1];
            break;
        case mirror::all:
            sprs[0]->img = sprite_imgs[top_left_index_in_16_sprites_row];
            sprs[0]->flip = 1; // horizontal
            sprs[1]->img = sprite_imgs[top_left_index_in_16_sprites_row];
            sprs[1]->flip = 2; // vertical
            sprs[2]->img = sprite_imgs[top_left_index_in_16_sprites_row];
            sprs[2]->flip = 3; // horizontal and vertical
            break;
        case mirror::horizontal:
            sprs[0]->img = sprite_imgs[top_left_index_in_16_sprites_row];
            sprs[0]->flip = 1; // horizontal
            sprs[1]->img = sprite_imgs[top_left_index_in_16_sprites_row + 16];
            sprs[2]->img = sprite_imgs[top_left_index_in_16_sprites_row + 16];
            sprs[2]->flip = 1; // horizontal
            break;
        case mirror::vertical:
            sprs[0]->img = sprite_imgs[top_left_index_in_16_sprites_row + 1];
            sprs[1]->img = sprite_imgs[top_left_index_in_16_sprites_row];
            sprs[1]->flip = 2; // vertical
            sprs[2]->img = sprite_imgs[top_left_index_in_16_sprites_row + 1];
            sprs[2]->flip = 2; // vertical
            break;
        }
    }

    ~sprites_2x2() {
        for (sprite* spr : sprs) {
            spr->img = nullptr;
            sprites.free(spr);
        }
    }

    auto pre_render(game_object* obj) -> void {
        obj->spr->scr_x = int16_t(obj->x - sprite_width);
        obj->spr->scr_y = int16_t(obj->y - sprite_height);
        sprs[0]->scr_x = int16_t(obj->x);
        sprs[0]->scr_y = int16_t(obj->y - sprite_height);
        sprs[1]->scr_x = int16_t(obj->x - sprite_width);
        sprs[1]->scr_y = int16_t(obj->y);
        sprs[2]->scr_x = int16_t(obj->x);
        sprs[2]->scr_y = int16_t(obj->y);
    }
};

static auto create_fragments(float const orig_x, float const orig_y,
                             int const count, float const speed,
                             clk::time const life_time_ms) -> void {
    for (int i = 0; i < count; ++i) {
        fragment* frg = new (objects.alloc()) fragment{};
        frg->die_at_ms = clk.ms + life_time_ms;
        frg->x = orig_x;
        frg->y = orig_y;
        frg->dx = random_float(-speed, speed);
        frg->dy = random_float(-speed, speed);
        frg->ddx = 2 * random_float(-speed, speed);
        frg->ddy = 2 * random_float(-speed, speed);
    }
}
