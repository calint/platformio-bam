#pragma once
// abstraction of the device used by 'main.cpp'

#include <SPI.h>
#include <SPIFFS.h>

class device {
public:
  virtual ~device() = default;

  // initiate device or abort
  virtual auto init() -> void = 0;

  // check if display is touched
  virtual auto display_is_touched() -> bool = 0;

  // if touched, get x, y with values between 0 and 4096 and pressure
  virtual auto display_get_touch(uint16_t &x, uint16_t &y,
                                 uint8_t &pressure) -> void = 0;

  // wait for previous DMA transaction to complete and make a new transaction
  virtual auto dma_write_bytes(uint8_t const *data, uint32_t len) -> void = 0;

  // returns true if DMA transaction is active
  virtual auto dma_is_busy() -> bool = 0;

  // wait for previous DMA transaction to complete or just return if none active
  virtual auto dma_wait_for_completion() -> void = 0;

  // read from SD a maximum of 'buf_len' into 'buf' from 'path'
  // returns number of bytes read or -1 if failed
  virtual auto sd_read(char const *path, char *buf, int buf_len) -> int {
    File file = SD.open(path);
    if (!file) {
      return -1;
    }
    const size_t n = file.read((uint8_t *)buf, buf_len);
    file.close();
    return n;
  }

  // write to SD 'buf_len' number of bytes from 'buf' to 'path'
  // returns true if ok
  virtual auto sd_write(char const *path, char const *buf, int buf_len,
                        char const *mode = FILE_WRITE) -> bool {
    File file = SD.open(path, mode);
    if (!file) {
      return false;
    }
    const size_t n = file.write((uint8_t *)buf, buf_len);
    const bool ok = n == buf_len;
    file.close();
    return ok;
  }

  // read from SPIFFS a maximum of 'buf_len' into 'buf' from 'path'
  // returns number of bytes read or -1 if failed
  virtual auto spiffs_read(char const *path, char *buf, int buf_len) -> int {
    File file = SPIFFS.open(path);
    if (!file) {
      return -1;
    }
    const size_t n = file.read((uint8_t *)buf, buf_len);
    file.close();
    return n;
  }

  // write to SPIFFS 'buf_len' number of bytes from 'buf' to 'path'
  // returns true if ok
  virtual auto spiffs_write(char const *path, char const *buf, int buf_len,
                            char const *mode = FILE_WRITE) -> bool {
    File file = SPIFFS.open(path, mode);
    if (!file) {
      return false;
    }
    const size_t n = file.write((uint8_t const *)buf, buf_len);
    file.close();
    return n == buf_len;
  }

protected:
  // initiate SD and SPIFFS, print if not available
  virtual auto init_sd_spiffs(SPIClass &spi, uint8_t sd_cs,
                              int sd_bus_freq = 4000000) -> void {
    if (!SD.begin(sd_cs, spi, sd_bus_freq)) {
      printf("* no SD card\n");
    }

    if (!SPIFFS.begin()) {
      printf("* no SPIFFS\n");
    }
  }
};