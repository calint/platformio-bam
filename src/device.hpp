#pragma once
// abstraction of the device needed by 'main.cpp'

class device {
public:
  virtual ~device() = default;
  virtual void init() = 0;
  virtual auto display_is_touched() -> bool = 0;
  virtual void display_get_touch(uint16_t &x, uint16_t &y,
                                 uint8_t &pressure) = 0;
  virtual void dma_write_bytes(uint8_t const *data, uint32_t len) = 0;
  virtual auto dma_is_busy() -> bool = 0;
  virtual void dma_wait_for_completion() = 0;
};