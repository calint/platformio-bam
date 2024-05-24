#pragma once
// abstraction of the device used by 'main.cpp'

// reviewed: 2024-05-22

#include <SD.h>
#include <SPI.h>
#include <SPIFFS.h>

class device {
public:
  virtual ~device() = default;

  // initiate device or abort
  virtual auto init() -> void = 0;

  // check if display is touched
  virtual auto display_is_touched() -> bool = 0;

  // if touched, get x, y values between 0 and 4095
  // pressure between 0 and 255
  virtual auto display_get_touch(uint16_t &x, uint16_t &y,
                                 uint8_t &pressure) -> void = 0;

  // wait for previous DMA transaction to complete and make a new transaction
  virtual auto dma_write_bytes(uint8_t const *data, uint32_t len) -> void = 0;

  // returns true if DMA transaction is active
  virtual auto dma_is_busy() -> bool = 0;

  // wait for previous DMA transaction to complete or just return if none active
  virtual auto dma_wait_for_completion() -> void = 0;

  // read from SD path 'path' maximum 'buf_len' into 'buf'
  // returns number of bytes read or -1 if failed
  virtual auto sd_read(char const *path, char *buf, int buf_len) -> int {
    return fs_read(SD, path, buf, buf_len);
  }

  // write to SD 'path' 'buf_len' bytes from 'buf', 'mode' "w" or "a" for write
  // or append
  // returns true if ok
  virtual auto sd_write(char const *path, char const *buf, int buf_len,
                        char const *mode) -> bool {
    return fs_write(SD, path, buf, buf_len, mode);
  }

  // read from SPIFFS 'path' maximum 'buf_len' into 'buf'
  // returns number of bytes read or -1 if failed
  virtual auto spiffs_read(char const *path, char *buf, int buf_len) -> int {
    return fs_read(SPIFFS, path, buf, buf_len);
  }

  // write to SPIFFS 'path' 'buf_len' bytes from 'buf', 'mode' "w" or "a" for
  // write or append
  // returns true if ok
  virtual auto spiffs_write(char const *path, char const *buf, int buf_len,
                            char const *mode) -> bool {
    return fs_write(SPIFFS, path, buf, buf_len, mode);
  }

  // returns true if SD card present and initiated
  auto sd_available() -> bool { return sd_present_; }

  // returns true is SPIFFS present and initiated
  auto spiffs_available() -> bool { return spiffs_present_; }

protected:
  // initiate SD and SPIFFS
  auto init_sd_spiffs(SPIClass &spi, uint8_t sd_cs,
                      int sd_bus_freq = 4000000) -> void {
    if (SD.begin(sd_cs, spi, sd_bus_freq)) {
      sd_present_ = true;
    }

    if (SPIFFS.begin()) {
      spiffs_present_ = true;
    }
  }

private:
  bool sd_present_ = false;
  bool spiffs_present_ = false;

  auto fs_write(FS &fs, char const *path, char const *buf, int const buf_len,
                char const *mode) -> bool {
    File file = fs.open(path, mode);
    if (!file) {
      return false;
    }
    size_t const n =
        file.write(reinterpret_cast<uint8_t const *>(buf), buf_len);
    file.close();
    return n == buf_len;
  }

  auto fs_read(FS &fs, char const *path, char *buf, int const buf_len) -> int {
    File file = fs.open(path);
    if (!file) {
      return -1;
    }
    size_t const n = file.read(reinterpret_cast<uint8_t *>(buf), buf_len);
    file.close();
    return n;
  }
};