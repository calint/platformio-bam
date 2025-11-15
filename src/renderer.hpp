#pragma once

#include "engine.hpp"

#include <cstring>

// defined in device implementation
extern int const display_width;
extern int const display_height;

// statistics about ratio of busy DMA before sending new buffer (higher is
// better meaning DMA is not finished before rendering)
static int dma_busy = 0;
static int dma_writes = 0;

// pixel precision collision detection between on screen sprites
// allocated in 'renderer_init()'
static size_t const collision_map_size_B =
    sizeof(sprite_ix) * display_width * display_height;
static sprite_ix* collision_map;

// forward declarations to used device functions
auto device_dma_write_bytes(uint8_t const* data, uint32_t len) -> void;
auto device_dma_is_busy() -> bool;
auto device_alloc_dma_buffer(size_t n) -> void*;
auto device_alloc_internal_buffer(size_t n) -> void*;

// number of scanlines to render before DMA transfer
static int constexpr dma_n_scanlines = 8;
// note: performance on device (scanlines:FPS):
//  ESP32-2432S028R:
//    1:23, 2:27, 4:29, 8:31, 16:31, 32:32, 64:32
//  JC4827W543R:
//    1:27, 2:35, 4:41, 8:44, 16:47, 32:48, 64:hw limit exceeded

// implements buffer swapping
class dma_buffers final {
  public:
    // size of a DMA buffer
    size_t const buf_size_B =
        sizeof(uint16_t) * display_width * dma_n_scanlines;

    auto init() -> void {
        buf_1_ = static_cast<uint16_t*>(device_alloc_dma_buffer(buf_size_B));
        buf_2_ = static_cast<uint16_t*>(device_alloc_dma_buffer(buf_size_B));
        if (!buf_1_ || !buf_2_) {
            printf("!!! could not allocate DMA buffers\n");
            exit(1);
        }
        buf_current_ = buf_1_;
    }

    auto current_buffer() -> uint16_t* { return buf_current_; }

    auto swap() -> uint16_t* {
        buf_current_ = buf_current_ == buf_1_ ? buf_2_ : buf_1_;
        return buf_current_;
    }

  private:
    uint16_t* buf_1_ = nullptr;
    uint16_t* buf_2_ = nullptr;
    uint16_t* buf_current_ = nullptr;
} static dma_buffers{};

inline auto renderer_init() -> void {
    dma_buffers.init();

    collision_map = static_cast<sprite_ix*>(device_alloc_internal_buffer(
        display_width * display_height * sizeof(sprite_ix)));
    if (!collision_map) {
        printf("!!! could not allocate collision map\n");
        exit(1);
    }
}

// sprites to be rendered divided in layers
struct render_sprite_entry {
    sprite const* spr = nullptr;
    sprite_ix ix = 0; // index in sprite array
};

// list of sprites to render by layer
static render_sprite_entry render_sprite_entries[sprite_layer_count]
                                                [sprite_count];
// pointers to end of list in 'render_sprite_entries'
static render_sprite_entry* render_sprite_entries_end[sprite_layer_count];

static inline auto printf_render_sprite_entries_ram_usage() -> void {
    printf("    render sprites: %zu B\n",
           sizeof(render_sprite_entries) + sizeof(render_sprite_entries_end));
}

// build lists of visible sprites based on layer index
// only used in 'render(...)'
static inline auto update_render_sprite_lists() -> void {
    // set end of lists pointers to start of lists
    for (int i = 0; i < sprite_layer_count; ++i) {
        render_sprite_entries_end[i] = &render_sprite_entries[i][0];
    }
    // build entries lists
    sprite const* spr = sprites.all_list();
    int const len = sprites.all_list_len();
    // note: "int constexpr len" does not compile
    for (int i = 0; i < len; ++i, ++spr) {
        if (!spr->img || spr->scr_x <= -sprite_width ||
            spr->scr_x >= display_width || spr->scr_y <= -sprite_height ||
            spr->scr_y >= display_height) {
            // sprite has no image or is outside the screen
            continue;
        }
        // create an entry at the end of the list of current sprite layer and
        // increase that pointer
        render_sprite_entry* rse = render_sprite_entries_end[spr->layer];
        rse->ix = sprite_ix(i);
        rse->spr = spr;
        ++render_sprite_entries_end[spr->layer];
    }
}

// renders a scanline
// note: inline because it is only called from one location in render(...)
static inline auto
render_scanline(uint16_t* render_buf_ptr, sprite_ix* collision_map_row_ptr,
                int tile_x, int tile_x_fract,
                tile_img_ix const* tile_map_row_ptr,
                uint8_t const* tile_map_flags_row_ptr, int16_t const scanline_y,
                int const tile_line_times_tile_width,
                int const tile_line_times_tile_width_flipped) -> void {

    // used later by sprite renderer to overwrite tile_imgs pixels
    uint16_t* scanline_ptr = render_buf_ptr;
    // pointer to first tile to render
    tile_img_ix const* tile_map_ptr = tile_map_row_ptr + tile_x;
    uint8_t const* tile_map_flags_ptr = tile_map_flags_row_ptr + tile_x;
    // for all horizontal pixels
    int remaining_x = display_width;
    while (remaining_x) {
        uint8_t tile_flags = *tile_map_flags_ptr;
        bool const flip_horiz = tile_flags & 0x8;
        bool const flip_vert = tile_flags & 0x4;
        // pointer to tile image to render
        uint8_t const* tile_img_ptr = &tile_imgs[*tile_map_ptr][0];
        if (flip_vert) {
            tile_img_ptr += tile_line_times_tile_width_flipped;
        } else {
            tile_img_ptr += tile_line_times_tile_width;
        }
        if (flip_horiz) {
            tile_img_ptr += tile_width - 1 - tile_x_fract;
        } else {
            tile_img_ptr += tile_x_fract;
        }
        int const tile_img_ptr_inc = flip_horiz ? -1 : 1;
        // calculate number of pixels to render
        int render_n_pixels = 0;
        if (tile_x_fract) {
            // can only happen at first tile in row
            render_n_pixels = tile_width - tile_x_fract;
            if (render_n_pixels > remaining_x) {
                // tile width + 1 is greater than screen width
                render_n_pixels = remaining_x;
            }
            tile_x_fract = 0;
        } else {
            render_n_pixels =
                remaining_x < tile_width ? remaining_x : tile_width;
        }
        // decrease remaining pixels to render before using that variable
        remaining_x -= render_n_pixels;
        while (render_n_pixels--) {
            *render_buf_ptr = palette_tiles[*tile_img_ptr];
            tile_img_ptr += tile_img_ptr_inc;
            ++render_buf_ptr;
        }
        // next tile
        ++tile_map_ptr;
        ++tile_map_flags_ptr;
    }

    // render sprites
    // note: although grossly inefficient algorithm the DMA is mostly busy while
    //       rendering

    for (int layer = 0; layer < sprite_layer_count; ++layer) {
        render_sprite_entry* spr_it_end = render_sprite_entries_end[layer];
        for (render_sprite_entry* spr_it = &render_sprite_entries[layer][0];
             spr_it < spr_it_end; ++spr_it) {
            sprite const* const spr = spr_it->spr;
            if (spr->scr_y > scanline_y ||
                spr->scr_y + sprite_height <= scanline_y) {
                // not within scanline
                continue;
            }
            // pointer to sprite image to be rendered
            uint8_t const* spr_img_ptr = spr->img;
            // extract sprite flip
            bool const flip_horiz = spr->flip & 1;
            bool const flip_vert = spr->flip & 2;
            if (flip_vert) {
                spr_img_ptr += (sprite_height - 1) * sprite_width -
                               (scanline_y - spr->scr_y) * sprite_width;
            } else {
                spr_img_ptr += (scanline_y - spr->scr_y) * sprite_width;
            }
            if (flip_horiz) {
                // start at end of sprite line
                spr_img_ptr += sprite_width - 1;
            }
            // increment to next sprite pixel to be rendered
            int const spr_img_ptr_inc = flip_horiz ? -1 : 1;
            // pointer to destination of sprite data
            uint16_t* scanline_dst_ptr = scanline_ptr + spr->scr_x;
            // initial number of pixels to be rendered
            int render_n_pixels = sprite_width;
            // pointer to collision map for first pixel of sprite
            sprite_ix* collision_pixel = collision_map_row_ptr + spr->scr_x;
            if (spr->scr_x < 0) {
                // adjustments if sprite x is negative
                if (flip_horiz) {
                    spr_img_ptr += spr->scr_x;
                } else {
                    spr_img_ptr -= spr->scr_x;
                }
                scanline_dst_ptr -= spr->scr_x;
                render_n_pixels += spr->scr_x;
                collision_pixel -= spr->scr_x;
            } else if (spr->scr_x + sprite_width > display_width) {
                // adjustment if sprite partially outside screen (x-wise)
                render_n_pixels = display_width - spr->scr_x;
            }
            // render line from sprite to scanline and check collisions
            object* obj = spr->obj;
            while (render_n_pixels--) {
                // write pixel from sprite data or skip if 0
                uint8_t const color_ix = *spr_img_ptr;
                if (color_ix) {
                    // if not transparent pixel
                    *scanline_dst_ptr = palette_sprites[color_ix];
                    if (*collision_pixel != sprite_ix_reserved) {
                        // if other sprite has written to this pixel
                        sprite* other_spr = sprites.instance(*collision_pixel);
                        if (spr->layer == other_spr->layer) {
                            object* other_obj = other_spr->obj;
                            if (obj->col_mask & other_obj->col_bits) {
                                obj->col_with = other_obj;
                            }
                            if (other_obj->col_mask & obj->col_bits) {
                                other_obj->col_with = obj;
                            }
                        }
                    }
                    // set pixel collision value to sprite index
                    *collision_pixel = spr_it->ix;
                }
                spr_img_ptr += spr_img_ptr_inc;
                ++collision_pixel;
                ++scanline_dst_ptr;
            }
        }
    }
}

// returns number of shifts to convert a 2^n number to 1
static auto constexpr count_right_shifts_until_1(int num) -> int {
    return (num <= 1) ? 0 : 1 + count_right_shifts_until_1(num >> 1);
}

// renders tile map and sprites
inline auto render(int const x, int const y) -> void {
    // clear stats for this frame
    dma_busy = dma_writes = 0;

    // clear collisions map
    // note: works on other sizes of type 'sprite_ix' because reserved value is
    //       unsigned maximum value such as 0xff or 0xffff etc
    memset(collision_map, sprite_ix_reserved, collision_map_size_B);

    // extract whole number and fractions from x, y
    int constexpr tile_width_shift = count_right_shifts_until_1(tile_width);
    int constexpr tile_height_shift = count_right_shifts_until_1(tile_height);
    int constexpr tile_width_and = (1 << tile_width_shift) - 1;
    int constexpr tile_height_and = (1 << tile_height_shift) - 1;
    int const tile_x = x >> tile_width_shift;
    int const tile_x_fract = x & tile_width_and;
    int tile_y = y >> tile_height_shift;
    int tile_y_fract = y & tile_height_and;
    // current scanline screen y
    int16_t scanline_y = 0;
    // pointer to start of current row of tiles
    tile_img_ix const* tile_map_row_ptr = &tile_map[tile_y][0];
    // pointer to start of current row of tile flags
    uint8_t const* tile_map_flags_row_ptr = &tile_map_flags[tile_y][0];
    // pointer to collision map starting at top left of screen
    sprite_ix* collision_map_row_ptr = collision_map;
    // keeps track of how many scanlines have been rendered since last DMA
    // transfer
    int dma_scanline_count = 0;
    // buffer to render
    uint16_t* render_buf_ptr = dma_buffers.current_buffer();
    // prepare visible sprites lists based on layer index
    update_render_sprite_lists();
    // for all lines on display
    int remaining_y = display_height;
    while (remaining_y) {
        // render from tiles map and sprites to the 'render_buf_ptr'
        int const render_n_tile_lines =
            remaining_y < tile_height ? remaining_y : tile_height;
        // prepare loop variables
        int render_n_scanlines = 0;
        int tile_line = 0;
        int tile_line_times_tile_width = 0;
        int tile_line_times_tile_width_flipped = 0;
        if (tile_y_fract) {
            // note: assumes display height is at least a tile height -1
            render_n_scanlines = tile_height - tile_y_fract;
            tile_line = tile_y_fract;
            tile_line_times_tile_width = tile_y_fract * tile_width;
            tile_line_times_tile_width_flipped =
                (tile_height - 1 - tile_y_fract) * tile_width;
            tile_y_fract = 0;
        } else {
            render_n_scanlines = render_n_tile_lines;
            tile_line_times_tile_width = tile_line = 0;
            tile_line_times_tile_width_flipped = (tile_height - 1) * tile_width;
        }
        // render a row from tile map
        while (tile_line < render_n_tile_lines) {
            render_scanline(
                render_buf_ptr, collision_map_row_ptr, tile_x, tile_x_fract,
                tile_map_row_ptr, tile_map_flags_row_ptr, scanline_y,
                tile_line_times_tile_width, tile_line_times_tile_width_flipped);
            ++tile_line;
            tile_line_times_tile_width += tile_width;
            tile_line_times_tile_width_flipped -= tile_width;
            render_buf_ptr += display_width;
            collision_map_row_ptr += display_width;
            ++scanline_y;
            ++dma_scanline_count;
            if (dma_scanline_count == dma_n_scanlines) {
                ++dma_writes;
                dma_busy += device_dma_is_busy() ? 1 : 0;
                device_dma_write_bytes(
                    reinterpret_cast<uint8_t*>(dma_buffers.current_buffer()),
                    uint32_t(display_width * dma_n_scanlines *
                             sizeof(uint16_t)));
                // swap to the other render buffer
                render_buf_ptr = dma_buffers.swap();
                dma_scanline_count = 0;
            }
        }
        ++tile_y;
        remaining_y -= render_n_scanlines;
        tile_map_row_ptr += tile_map_width;
        tile_map_flags_row_ptr += tile_map_width;
    }
    // if 'display_height' is not evenly divisible by 'n_scanlines' there
    // will be remaining scanlines to write
    int const dma_n_scanlines_trailing = display_height % dma_n_scanlines;
    if (dma_n_scanlines_trailing) {
        ++dma_writes;
        dma_busy += device_dma_is_busy() ? 1 : 0;
        device_dma_write_bytes(
            reinterpret_cast<uint8_t*>(dma_buffers.current_buffer()),
            uint32_t(display_width * dma_n_scanlines_trailing *
                     sizeof(uint16_t)));
        // swap to the other render buffer
        dma_buffers.swap();
    }
}
