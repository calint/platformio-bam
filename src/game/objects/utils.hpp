#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "fragment.hpp"

class sprites_2x2 final {
  // three additional sprites
  sprite *sprs[3];

public:
  sprites_2x2(game_object *obj, const int top_left_index_in_16_sprites_row,
              const uint8_t layer) {
    obj->spr = sprites.allocate_instance();
    obj->spr->obj = obj;
    obj->spr->layer = layer;
    obj->spr->flip = 0;
    // additional 3 sprites
    for (int i = 0; i < 3; i++) {
      sprs[i] = sprites.allocate_instance();
      sprs[i]->obj = obj;
      sprs[i]->layer = layer;
      sprs[i]->flip = 0;
    }
    obj->spr->img = sprite_imgs[top_left_index_in_16_sprites_row];
    sprs[0]->img = sprite_imgs[top_left_index_in_16_sprites_row + 1];
    sprs[1]->img = sprite_imgs[top_left_index_in_16_sprites_row + 16];
    sprs[2]->img = sprite_imgs[top_left_index_in_16_sprites_row + 16 + 1];
  }

  ~sprites_2x2() {
    for (int i = 0; i < 3; i++) {
      sprs[i]->img = nullptr;
      sprites.free_instance(sprs[i]);
    }
  }

  void pre_render(game_object *obj) {
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

static void create_fragments(const float orig_x, const float orig_y,
                             const int count, const float speed,
                             const clk::time life_time_ms) {
  for (int i = 0; i < count; i++) {
    fragment *frg = new (objects.allocate_instance()) fragment{};
    frg->die_at_ms = clk.ms + life_time_ms;
    frg->x = orig_x;
    frg->y = orig_y;
    frg->dx = random_float(-speed, speed);
    frg->dy = random_float(-speed, speed);
    frg->ddx = 2 * random_float(-speed, speed);
    frg->ddy = 2 * random_float(-speed, speed);
  }
}
