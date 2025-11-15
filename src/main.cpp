//
// bam game platform
//

// note: design decision of 'hpp' source files
// * the program is one file split into logical sections using includes
// * globals are declared 'static'
// * increases optimization opportunities for the compiler
// * directory 'game' contains the user code that interfaces with 'engine.hpp'
// * order of include and content of 'defs.hpp', 'state.hpp', 'application.hpp'
//   solves circular references and gives user the necessary callbacks to
//   interface with engine

// note: design decision regarding 'unsigned' - due to sign conversion warnings
// and subtle bugs in mixed signedness operations, signed constants and
// variables are used where the bit width of the type is wide enough to fit the
// largest value

// note: const and constexpr declarations are right-to-left convention

// note: increments and decrements done prefix for compatibility with iterators

// note: auto is used when type declaration is too verbose such as iterators

// note: why some buffers are allocated at 'setup'
// Due to a technical limitation, the maximum statically allocated DRAM usage is
// 160KB. The remaining 160KB (for a total of 320KB of DRAM) can only be
// allocated at runtime as heap.
// -- https://stackoverflow.com/questions/71085927/how-to-extend-esp32-heap-size

// note: esp32 s3 can allocate more than 160 KB

// reviewed: 2023-12-11
// reviewed: 2024-05-01
// reviewed: 2024-05-22

#include <Arduino.h>
#include <hal/efuse_hal.h>

// main entry file to user code
#include "application/application.hpp"
// the common renderer
#include "renderer.hpp"

// device file specified in `platformio.ini`
#if DEVICE_ESP32_2432S028R
#include "devices/ESP32_2432S028R.hpp"
static ESP32_2432S028R device{};

#elif DEVICE_JC4827W543R
#include "devices/JC4827W543R.hpp"
static JC4827W543R device{};

#elif DEVICE_JC4827W543C
#include "devices/JC4827W543C.hpp"
static JC4827W543C device{};

#else
#error                                                                         \
    "None of known devices defined: DEVICE_JC4827W543R, DEVICE_JC4827W543C, DEVICE_ESP32_2432S028R"
#endif

// functions used in `renderer.hpp` to decouple from device implementation
auto device_dma_write_bytes(uint8_t const* data, uint32_t len) -> void {
    device.dma_write_bytes(data, len);
}
auto device_dma_is_busy() -> bool { return device.dma_is_busy(); };
auto device_alloc_dma_buffer(uint32_t n) -> void* {
    return heap_caps_calloc(1, n, MALLOC_CAP_DMA);
}
auto device_alloc_internal_buffer(uint32_t n) -> void* {
    return heap_caps_calloc(1, n, MALLOC_CAP_INTERNAL);
}
// --

auto setup() -> void {
    Serial.begin(115200);
    delay(500); // arbitrary wait for serial to connect

    heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);

    printf("------------------- platform -----------------------------\n");
    printf("        chip model: %s\n", ESP.getChipModel());
    printf("          revision: %u.%u\n", efuse_hal_get_major_chip_version(),
           efuse_hal_get_minor_chip_version());
    printf("             cores: %u\n", ESP.getChipCores());
    printf("              freq: %u MHz\n", ESP.getCpuFreqMHz());
    printf("           esp-idf: %s\n", esp_get_idf_version());
    printf("            screen: %u x %u px\n", display_width, display_height);
    printf("     free heap mem: %u B\n", ESP.getFreeHeap());
    printf("largest free block: %u B\n", ESP.getMaxAllocHeap());
    printf("------------------- object sizes -------------------------\n");
    printf("            sprite: %zu B\n", sizeof(sprite));
    printf("            object: %zu B\n", sizeof(object));
    printf("   object instance: %d B\n", object_instance_max_size_B);
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
    clk.init(millis(), clk_fps_update_ms, clk_locked_dt_ms);
    // note: not in 'engine_init()' due to dependency on 'millis()'

    engine_init();

    application_init();

    printf("------------------- on heap ------------------------------\n");
    printf("   DMA buf 1 and 2: %u B\n", 2 * dma_buffers.buf_size_B);
    printf("      sprites data: %d B\n", sprites.allocated_data_size_B());
    printf("      objects data: %d B\n", objects.allocated_data_size_B());
    printf("     collision map: %u B\n", collision_map_size_B);
    printf("------------------- after setup --------------------------\n");
    printf("     free heap mem: %u B\n", ESP.getFreeHeap());
    printf("largest free block: %u B\n", ESP.getMaxAllocHeap());
    printf("----------------------------------------------------------\n");

    heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
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
