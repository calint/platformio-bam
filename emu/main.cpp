//
// emulator of device with variable screen size and performance
// see `make.sh` for supplied defines
//

// note: rendering is kept as the implementation for closest emulation

// first the application defs
#include "../src/application/defs.hpp"
// then the device
#include "device_sdl.hpp"
// then the common renderer
#include "../src/renderer.hpp"
// then the main entry file to user code
#include "../src/application/application.hpp"

// instantiate the device implementation
static device_sdl device;

// arduino replacement
auto millis() -> unsigned long { return SDL_GetTicks(); }

// functions used in `renderer.hpp` to decouple from device implementation
auto device_dma_write_bytes(uint8_t const* data, uint32_t len) -> void {
    device.dma_write_bytes(data, len);
}
auto device_dma_is_busy() -> bool { return false; };
auto device_alloc_dma_buffer(size_t n) -> void* { return calloc(1, n); }
auto device_alloc_internal_buffer(size_t n) -> void* { return calloc(1, n); }

auto setup() -> void {
    printf("------------------- object sizes -------------------------\n");
    printf("            sprite: %zu B\n", sizeof(sprite));
    printf("            object: %zu B\n", sizeof(object));
    printf("   object instance: %zu B\n", size_t(object_instance_max_size_B));
    printf("              tile: %zu B\n", sizeof(tile_imgs[0]));
    printf("------------------- in program memory --------------------\n");
    printf("     sprite images: %zu B\n", sizeof(sprite_imgs));
    printf("       tile images: %zu B\n", sizeof(tile_imgs));
    printf("------------------- globals ------------------------------\n");
    printf("          tile map: %zu B\n", sizeof(tile_map));
    printf("    tile map flags: %zu B\n", sizeof(tile_map_flags));
    printf("           sprites: %zu B\n", sizeof(sprites));
    printf("           objects: %zu B\n", sizeof(objects));

    printf_render_sprite_entries_ram_usage();

    device.init();

    printf("------------------- peripherals --------------------------\n");
    printf("           SD card: %s\n",
           device.sd_available() ? "present" : "n/a");
    printf("              size: %zu B\n", device.sd_size_B());
    printf("              used: %zu B\n", device.sd_used_B());
    printf("            SPIFFS: %s\n",
           device.spiffs_available() ? "present" : "n/a");
    printf("              size: %zu B\n", device.spiffs_size_B());
    printf("              used: %zu B\n", device.spiffs_used_B());

    renderer_init();

    // initiate clock
    clk.init(clk::time(millis()), clk_fps_update_ms, clk_locked_dt_ms);
    // note: not in 'engine_init()' due to dependency on 'millis()'

    engine_init();

    application_init();

    printf("------------------- on heap ------------------------------\n");
    printf("   DMA buf 1 and 2: %zu B\n", 2 * dma_buffers.buf_size_B);
    printf("      sprites data: %d B\n", sprites.allocated_data_size_B());
    printf("      objects data: %d B\n", objects.allocated_data_size_B());
    printf("     collision map: %zu B\n", collision_map_size_B);
    printf("------------------- after setup --------------------------\n");
}

auto loop() -> void {
    if (clk.on_frame(clk::time(millis()))) {
        // note: not in 'engine_loop()' due to dependency on 'millis()'
        printf("t=%06u  fps=%02d  dma=%03d  objs=%03d  sprs=%03d\n", clk.ms,
               clk.fps, dma_busy * 100 / (dma_writes ? dma_writes : 1),
               objects.allocated_list_len(), sprites.allocated_list_len());
    }

    if (device.display_is_touched()) {
        uint8_t const touch_count = device.display_touch_count();
        device::touch touches[10]{};
        device.display_get_touch(touches);
        application_on_touch(touches, touch_count);
    }

    engine_loop();
}

auto main() -> int {
    setup();
    while (true) {
        unsigned long t0 = millis();
        loop();
        unsigned long dt = millis() - t0;
        if (BAM_TIME_STEP_MS > dt) {
            SDL_Delay(Uint32(BAM_TIME_STEP_MS - dt));
        }
    }
}
