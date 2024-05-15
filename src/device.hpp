#pragma once
// abstraction of the device needed by 'main.cpp'

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
};