//
//  JC4827W543R
//  Guition ESP32-S3, 8M PSRAM, 4M FLASH, WIFI, Bluetooth
//  4.3" IPS 480*272 LCD display
//
//  purchased at: https://www.aliexpress.com/item/1005006729377800.html
//          docs: http://pan.jczn1688.com/s/zyojx8
//

// note. design decision of 'hpp' source files
// * the program is one file split into logical sections using includes
// * all globals are declared 'static'
// * increases optimization opportunities for the compiler
// * directory 'game' contains the user code that interfaces with 'engine.hpp'
// * order of include and content of 'defs.hpp', 'game_state.hpp', 'main.hpp'
//   solves circular references and gives user the necessary callbacks to
//   interface with engine

// note. design decision regarding 'unsigned' - due to sign conversion warnings
// and subtle bugs in mixed signedness operations, signed constants and
// variables are used where the bit width of the type is wide enough to fit the
// largest value

// note. why some buffers are allocated at 'setup'
// Due to a technical limitation, the maximum statically allocated DRAM usage is
// 160KB. The remaining 160KB (for a total of 320KB of DRAM) can only be
// allocated at runtime as heap.
// -- https://stackoverflow.com/questions/71085927/how-to-extend-esp32-heap-size

// reviewed: 2023-12-11
// reviewed: 2024-05-01

#include <Arduino.h>
#include "hal/efuse_hal.h"

// first game defines
#include "game/defs.hpp"

// then platform constants
#include "platform.hpp"

// then the engine
#include "engine.hpp"

// then the main entry file to user code
#include "game/main.hpp"

// setup device
#if defined(DEVICE_JC4827W543R)
#include "devices/JC4827W543R.hpp"
static JC4827W543R device{};
#elif defined(DEVICE_ESP32_2432S028R)
#include "devices/ESP32_2432S028R.hpp"
static ESP32_2432S028R device{};
#else
#error                                                                         \
    "None of known devices defined: DEVICE_JC4827W543R, DEVICE_ESP32_2432S028R"
#endif

// number of scanlines to render before DMA transfer
static constexpr int dma_n_scanlines = 8;
// note. performance on device: max 60 (32640 B buffer) (max on device 32768 B)
//  ESP32-2432S028R:
//    1: 25 fps, 2: 28 fps, 4: 29 fps, 8: 31 fps, 16: 31 fps, 32: 31 fps
//  JC4827W543R:
//    1: 26 fps, 2: 32 fps, 4: 35 fps, 8: 37 fps, 16: 38 fps, 32: 39 fps

// alternating buffers for rendering scanlines while DMA is active
// allocated in 'setup()'
static constexpr int dma_buf_size_B =
    sizeof(uint16_t) * display_width * dma_n_scanlines;
static uint16_t *dma_buf_1 = nullptr;
static uint16_t *dma_buf_2 = nullptr;

// pixel precision collision detection between on screen sprites
// allocated in 'setup()'
static constexpr int collision_map_size_B =
    sizeof(sprite_ix) * display_width * display_height;
static sprite_ix *collision_map = nullptr;

// statistics about ratio of busy DMA before sending new buffer (higher is
// better meaning DMA is not finished before rendering)
static int dma_busy = 0;
static int dma_writes = 0;

void setup() {
  Serial.begin(115200);
  sleep(1); // arbitrary wait for serial to connects

  printf("\n\n");

  heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);

  printf("\n\n");
  printf("------------------- platform -----------------------------\n");
  printf("        chip model: %s\n", ESP.getChipModel());
  printf("          revision: %u.%u\n", efuse_hal_get_major_chip_version(),
         efuse_hal_get_minor_chip_version());
  printf("             cores: %u\n", ESP.getChipCores());
  printf("              freq: %u MHz\n", ESP.getCpuFreqMHz());
  printf("            screen: %u x %u px\n", display_width, display_height);
  printf("     free heap mem: %u B\n", ESP.getFreeHeap());
  printf("largest free block: %u B\n", ESP.getMaxAllocHeap());
  printf("------------------- type sizes ---------------------------\n");
  printf("              bool: %zu B\n", sizeof(bool));
  printf("              char: %zu B\n", sizeof(char));
  printf("             short: %zu B\n", sizeof(short));
  printf("               int: %zu B\n", sizeof(int));
  printf("              long: %zu B\n", sizeof(long));
  printf("         long long: %zu B\n", sizeof(long long));
  printf("             float: %zu B\n", sizeof(float));
  printf("            double: %zu B\n", sizeof(double));
  printf("             void*: %zu B\n", sizeof(void *));
  printf("------------------- object sizes -------------------------\n");
  printf("            sprite: %zu B\n", sizeof(sprite));
  printf("            object: %zu B\n", sizeof(object));
  printf("              tile: %zu B\n", sizeof(tiles[0]));
  printf("------------------- in program memory --------------------\n");
  printf("     sprite images: %zu B\n", sizeof(sprite_imgs));
  printf("             tiles: %zu B\n", sizeof(tiles));
  printf("------------------- globals ------------------------------\n");
  printf("          tile map: %zu B\n", sizeof(tile_map));
  printf("           sprites: %zu B\n", sizeof(sprites));
  printf("           objects: %zu B\n", sizeof(objects));

  device.init();

  dma_buf_1 = static_cast<uint16_t *>(
      heap_caps_calloc(1, dma_buf_size_B, MALLOC_CAP_DMA));
  dma_buf_2 = static_cast<uint16_t *>(
      heap_caps_calloc(1, dma_buf_size_B, MALLOC_CAP_DMA));
  if (!dma_buf_1 || !dma_buf_2) {
    printf("!!! could not allocate DMA buffers\n");
    exit(1);
  }

  collision_map = static_cast<sprite_ix *>(heap_caps_calloc(
      display_width * display_height, sizeof(sprite_ix), MALLOC_CAP_INTERNAL));
  if (!collision_map) {
    printf("!!! could not allocate collision map\n");
    exit(1);
  }

  // initiate clock
  clk.init(millis(), clk_fps_update_ms, clk_locked_dt_ms);
  // note. not in 'engine_init()' due to dependency on 'millis()'

  engine_init();

  main_init();

  printf("------------------- on heap ------------------------------\n");
  printf("   DMA buf 1 and 2: %d B\n", 2 * dma_buf_size_B);
  printf("      sprites data: %d B\n", sprites.allocated_data_size_B());
  printf("      objects data: %d B\n", objects.allocated_data_size_B());
  printf("     collision map: %d B\n", collision_map_size_B);
  printf("------------------- after setup --------------------------\n");
  printf("     free heap mem: %u B\n", ESP.getFreeHeap());
  printf("largest free block: %u B\n", ESP.getMaxAllocHeap());
  printf("----------------------------------------------------------\n");

  heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
}

void loop() {
  if (clk.on_frame(clk::time(millis()))) {
    // note. not in 'engine_loop()' due to dependency on 'millis()'
    printf("t=%06u  fps=%02d  dma=%03d  objs=%03d  sprs=%03d\n", clk.ms,
           clk.fps, dma_busy * 100 / dma_writes, objects.allocated_list_len(),
           sprites.allocated_list_len());
  }

  if (device.is_display_touched()) {
    int16_t x = 0;
    int16_t y = 0;
    int16_t z = 0;
    device.get_display_touch(x, y, z);
    // ESP_LOGI("b", "x=%d  y=%d  z=%d", x, y, z);
    main_on_touch(x, y, z);
  }

  engine_loop();
}

// renders a scanline
// note. inline because it is only called from one location in render(...)
static inline void render_scanline(uint16_t *render_buf_ptr,
                                   sprite_ix *collision_map_row_ptr, int tile_x,
                                   int tile_x_fract,
                                   tile_ix const *tiles_map_row_ptr,
                                   const int16_t scanline_y,
                                   const int tile_line_times_tile_width) {

  // used later by sprite renderer to overwrite tiles pixels
  uint16_t *scanline_ptr = render_buf_ptr;
  // pointer to first tile to render
  tile_ix const *tiles_map_ptr = tiles_map_row_ptr + tile_x;
  // for all horizontal pixels
  int remaining_x = display_width;
  while (remaining_x) {
    // pointer to tile image to render
    uint8_t const *tile_img_ptr =
        tiles[*tiles_map_ptr] + tile_line_times_tile_width + tile_x_fract;
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
      render_n_pixels = remaining_x < tile_width ? remaining_x : tile_width;
    }
    // decrease remaining pixels to render before using that variable
    remaining_x -= render_n_pixels;
    while (render_n_pixels--) {
      *render_buf_ptr++ = palette_tiles[*tile_img_ptr++];
    }
    // next tile
    tiles_map_ptr++;
  }

  // render sprites
  // note. although grossly inefficient algorithm the DMA is mostly busy while
  //       rendering

  for (int layer = 0; layer < sprites_layers; layer++) {
    sprite *spr = sprites.all_list();
    const int len = sprites.all_list_len();
    // note. "constexpr int len" does not compile
    for (int i = 0; i < len; i++, spr++) {
      if (spr->layer != layer || !spr->img || spr->scr_y > scanline_y ||
          spr->scr_y + sprite_height <= scanline_y ||
          spr->scr_x <= -sprite_width || spr->scr_x >= display_width) {
        // sprite not in current layer or
        // sprite has no image or
        // not within scanline or
        // is outside the screen x-wise
        continue;
      }
      // pointer to sprite image to be rendered
      uint8_t const *spr_img_ptr = spr->img;
      // extract sprite flip
      const bool flip_horiz = spr->flip & 1;
      const bool flip_vert = spr->flip & 2;
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
      const int spr_img_ptr_inc = flip_horiz ? -1 : 1;
      // pointer to destination of sprite data
      uint16_t *scanline_dst_ptr = scanline_ptr + spr->scr_x;
      // initial number of pixels to be rendered
      int render_n_pixels = sprite_width;
      // pointer to collision map for first pixel of sprite
      sprite_ix *collision_pixel = collision_map_row_ptr + spr->scr_x;
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
      object *obj = spr->obj;
      while (render_n_pixels--) {
        // write pixel from sprite data or skip if 0
        const uint8_t color_ix = *spr_img_ptr;
        if (color_ix) {
          // if not transparent pixel
          *scanline_dst_ptr = palette_sprites[color_ix];
          if (*collision_pixel != sprite_ix_reserved) {
            // if other sprite has written to this pixel
            sprite *other_spr = sprites.instance(*collision_pixel);
            if (spr->layer == other_spr->layer) {
              object *other_obj = other_spr->obj;
              if (obj->col_mask & other_obj->col_bits) {
                obj->col_with = other_obj;
              }
              if (other_obj->col_mask & obj->col_bits) {
                other_obj->col_with = obj;
              }
            }
          }
          // set pixel collision value to sprite index
          *collision_pixel = sprite_ix(i);
        }
        spr_img_ptr += spr_img_ptr_inc;
        collision_pixel++;
        scanline_dst_ptr++;
      }
    }
  }
}

// returns number of shifts to convert a 2^n number to 1
static constexpr int count_right_shifts_until_1(int num) {
  return (num <= 1) ? 0 : 1 + count_right_shifts_until_1(num >> 1);
}

// renders tile map and sprites
static void render(const int x, const int y) {
  dma_busy = dma_writes = 0;

  // clear collisions map
  // note. works on other sizes of type 'sprite_ix' because reserved value is
  //       unsigned maximum value such as 0xff or 0xffff etc
  memset(collision_map, sprite_ix_reserved, collision_map_size_B);

  // extract whole number and fractions from x, y
  constexpr int tile_width_shift = count_right_shifts_until_1(tile_width);
  constexpr int tile_height_shift = count_right_shifts_until_1(tile_height);
  constexpr int tile_width_and = (1 << tile_width_shift) - 1;
  constexpr int tile_height_and = (1 << tile_height_shift) - 1;
  const int tile_x = x >> tile_width_shift;
  const int tile_x_fract = x & tile_width_and;
  int tile_y = y >> tile_height_shift;
  int tile_y_fract = y & tile_height_and;
  // current scanline screen y
  int16_t scanline_y = 0;
  // pointer to start of current row of tiles
  tile_ix const *tiles_map_row_ptr = tile_map[tile_y];
  // pointer to collision map starting at top left of screen
  sprite_ix *collision_map_row_ptr = collision_map;
  // keeps track of how many scanlines have been rendered since last DMA
  // transfer
  int dma_scanline_count = 0;
  // select first buffer for rendering
  uint16_t *render_buf_ptr = dma_buf_1;
  // which dma buffer to use next
  bool dma_buf_use_first = false;
  // pointer to the buffer that DMA will copy to screen
  uint16_t *dma_buf = render_buf_ptr;
  // for all lines on display
  int remaining_y = display_height;
  while (remaining_y) {
    // render from tiles map and sprites to the 'render_buf_ptr'
    int render_n_tile_lines =
        remaining_y < tile_height ? remaining_y : tile_height;
    // prepare loop variables
    int render_n_scanlines = 0;
    int tile_line = 0;
    int tile_line_times_tile_width = 0;
    if (tile_y_fract) {
      // note. assumes display height is at least a tile height -1
      render_n_scanlines = tile_height - tile_y_fract;
      tile_line = tile_y_fract;
      tile_line_times_tile_width = tile_y_fract * tile_height;
      tile_y_fract = 0;
    } else {
      render_n_scanlines = render_n_tile_lines;
      tile_line_times_tile_width = tile_line = 0;
    }
    // render a row from tile map
    while (tile_line < render_n_tile_lines) {
      render_scanline(render_buf_ptr, collision_map_row_ptr, tile_x,
                      tile_x_fract, tiles_map_row_ptr, scanline_y,
                      tile_line_times_tile_width);
      tile_line++;
      tile_line_times_tile_width += tile_width;
      render_buf_ptr += display_width;
      collision_map_row_ptr += display_width;
      scanline_y++;
      dma_scanline_count++;
      if (dma_scanline_count == dma_n_scanlines) {
        dma_writes++;
        dma_busy += device.asyncDMAIsBusy() ? 1 : 0;
        device.asyncDMAWriteBytes(
            reinterpret_cast<uint8_t *>(dma_buf),
            uint32_t(display_width * dma_n_scanlines * sizeof(uint16_t)));
        // bus.writeBytes(
        //     reinterpret_cast<uint8_t *>(dma_buf),
        //     uint32_t(display_width * dma_n_scanlines * sizeof(uint16_t)));
        dma_scanline_count = 0;
        // swap to the other render buffer
        dma_buf = render_buf_ptr = dma_buf_use_first ? dma_buf_1 : dma_buf_2;
        dma_buf_use_first = !dma_buf_use_first;
      }
    }
    tile_y++;
    remaining_y -= render_n_scanlines;
    tiles_map_row_ptr += tile_map_width;
  }
  // if 'display_height' is not evenly divisible by 'dma_n_scanlines' there will
  // be remaining scanlines to write
  constexpr int dma_n_scanlines_trailing = display_height % dma_n_scanlines;
  if (dma_n_scanlines_trailing) {
    dma_writes++;
    dma_busy += device.asyncDMAIsBusy() ? 1 : 0;
    device.asyncDMAWriteBytes(
        reinterpret_cast<uint8_t *>(dma_buf),
        uint32_t(display_width * dma_n_scanlines * sizeof(uint16_t)));
    // bus.writeBytes(
    //     reinterpret_cast<uint8_t *>(dma_buf),
    //     uint32_t(display_width * dma_n_scanlines * sizeof(uint16_t)));
  }
}
