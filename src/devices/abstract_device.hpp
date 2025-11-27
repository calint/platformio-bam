#pragma once
// abstraction of the device used by 'main.cpp' and implemented in `src/devices`

// note: device implementations must define global constants:
//    `static int32_t constexpr display_width`
//    `static int32_t constexpr display_height`

// reviewed: 2025-11-27

#include "../device.hpp"

#include <SD.h>
#include <SPI.h>
#include <SPIFFS.h>

class abstract_device : public device {
  public:
    // returns true is SPIFFS present and initiated
    auto spiffs_available() const -> bool override { return spiffs_present_; }

    // read from SPIFFS 'path' maximum 'buf_len' into 'buf'
    // returns number of bytes read or 0 if failed
    auto spiffs_read(char const* path, char* buf, size_t buf_len) const
        -> size_t override {
        return fs_read(SPIFFS, path, buf, buf_len);
    }

    // write to SPIFFS 'path' 'buf_len' bytes from 'buf', 'mode' "w" or "a" for
    // write or append
    // returns true if ok
    auto spiffs_write(char const* path, char const* buf, size_t buf_len,
                      char const* mode) const -> bool override {
        return fs_write(SPIFFS, path, buf, buf_len, mode);
    }

    // returns total size of SPIFFS in bytes or 0 if none present
    auto spiffs_size_B() const -> size_t override {
        return spiffs_present_ ? SPIFFS.totalBytes() : 0;
    }

    // returns number of used bytes on SPIFFS or 0 if none present
    auto spiffs_used_B() const -> size_t override {
        return spiffs_present_ ? SPIFFS.usedBytes() : 0;
    }

    // returns true if path exists
    auto spiffs_path_exists(char const* path) const -> bool override {
        return spiffs_present_ ? SPIFFS.exists(path) : false;
    }

    // returns true if SD card present and initiated
    auto sd_available() const -> bool override { return sd_present_; }

    // read from SD path 'path' maximum 'buf_len' into 'buf'
    // returns number of bytes read or 0 if failed
    auto sd_read(char const* path, char* buf, size_t buf_len) const
        -> size_t override {
        return fs_read(SD, path, buf, buf_len);
    }

    // write to SD 'path' 'buf_len' bytes from 'buf', 'mode' "w" or "a" for
    // write or append
    // returns true if ok
    auto sd_write(char const* path, char const* buf, size_t buf_len,
                  char const* mode) const -> bool override {
        return fs_write(SD, path, buf, buf_len, mode);
    }

    // returns total size of SD card in bytes or 0 if none present
    auto sd_size_B() const -> size_t override {
        return sd_present_ ? SD.totalBytes() : 0;
    }

    // returns number of bytes used on SD card or 0 if none present
    auto sd_used_B() const -> size_t override {
        return sd_present_ ? SD.usedBytes() : 0;
    }

    // returns true if path exists
    auto sd_path_exists(char const* path) const -> bool override {
        return sd_present_ ? SD.exists(path) : false;
    }

  protected:
    // initiate SD and SPIFFS
    auto init_sd_spiffs(SPIClass& spi, uint8_t sd_cs,
                        int32_t sd_bus_freq = 4000000) -> void {

        if (SD.begin(sd_cs, spi, sd_bus_freq)) {
            sd_present_ = true;
        }

        if (SPIFFS.begin(true)) {
            spiffs_present_ = true;
        }
    }

  private:
    bool sd_present_{};
    bool spiffs_present_{};

    auto fs_write(FS& fs, char const* path, char const* buf,
                  size_t const buf_len, char const* mode) const -> bool {

        File file = fs.open(path, mode);
        if (!file) {
            return false;
        }
        size_t const n =
            file.write(reinterpret_cast<uint8_t const*>(buf), buf_len);
        file.close();
        return n == buf_len;
    }

    auto fs_read(FS& fs, char const* path, char* buf,
                 size_t const buf_len) const -> size_t {

        File file = fs.open(path);
        if (!file) {
            return 0;
        }
        size_t const n = file.read(reinterpret_cast<uint8_t*>(buf), buf_len);
        file.close();
        return n;
    }

    auto fs_path_exists(FS& fs, char const* path) const -> bool {
        return fs.exists(path);
    }
};
