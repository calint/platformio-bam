#pragma once
// device interface used by 'main.cpp' and implemented in `src/devices`

// note: device implementations must define global constants:
//       `static int const display_width`
//       `static int const display_height`

// reviewed: 2024-05-22

#include <cstddef>
#include <cstdint>

class device {
  public:
    struct touch {
        int16_t x;
        int16_t y;
        uint8_t pressure;
    };

    virtual ~device() = default;

    // initiate device or abort
    virtual auto init() -> void = 0;

    // check if display is touched
    virtual auto display_is_touched() -> bool = 0;

    // number of touches read at `display_get_touch`
    virtual auto display_touch_count() -> uint8_t = 0;

    // if touched, get x, y values between 0 and 4095
    // pressure between 0 and 255
    virtual auto display_get_touch(touch touches[]) -> void = 0;

    // wait for previous DMA transaction to complete and make a new transaction
    virtual auto dma_write_bytes(uint8_t const* data, uint32_t len) -> void = 0;

    // returns true if DMA transaction is active
    virtual auto dma_is_busy() -> bool = 0;

    // wait for previous DMA transaction to complete or just return if none
    // active
    virtual auto dma_wait_for_completion() -> void = 0;

    // returns true is SPIFFS present and initiated
    virtual auto spiffs_available() const -> bool = 0;

    // read from SPIFFS 'path' maximum 'buf_len' into 'buf'
    // returns number of bytes read or -1 if failed
    virtual auto spiffs_read(char const* path, char* buf, int buf_len) const
        -> int = 0;

    // write to SPIFFS 'path' 'buf_len' bytes from 'buf', 'mode' "w" or "a" for
    // write or append
    // returns true if ok
    virtual auto spiffs_write(char const* path, char const* buf, int buf_len,
                              char const* mode) const -> bool = 0;

    // returns total size of SPIFFS in bytes or 0 if none present
    virtual auto spiffs_size_B() const -> size_t = 0;

    // returns number of used bytes on SPIFFS or 0 if none present
    virtual auto spiffs_used_B() const -> size_t = 0;

    // returns true if path exists
    virtual auto spiffs_path_exists(char const* path) const -> bool = 0;

    // returns true if SD card present and initiated
    virtual auto sd_available() const -> bool = 0;

    // read from SD path 'path' maximum 'buf_len' into 'buf'
    // returns number of bytes read or -1 if failed
    virtual auto sd_read(char const* path, char* buf, int buf_len) const
        -> int = 0;

    // write to SD 'path' 'buf_len' bytes from 'buf', 'mode' "w" or "a" for
    // write or append returns true if ok
    virtual auto sd_write(char const* path, char const* buf, int buf_len,
                          char const* mode) const -> bool = 0;

    // returns total size of SD card in bytes or 0 if none present
    virtual auto sd_size_B() const -> size_t = 0;

    // returns number of bytes used on SD card or 0 if none present
    virtual auto sd_used_B() const -> size_t = 0;

    // returns true if path exists
    virtual auto sd_path_exists(char const* path) const -> bool = 0;
};
