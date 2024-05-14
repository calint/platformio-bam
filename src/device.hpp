#pragma once
// abstraction of the device needed by 'main.cpp'

class device {
public:
  virtual ~device() = default;
  virtual auto init() -> void = 0;
  virtual auto display_is_touched() -> bool = 0;
  virtual auto display_get_touch(uint16_t &x, uint16_t &y,
                                 uint8_t &pressure) -> void = 0;
  virtual auto dma_write_bytes(uint8_t const *data, uint32_t len) -> void = 0;
  virtual auto dma_is_busy() -> bool = 0;
  virtual auto dma_wait_for_completion() -> void = 0;
};