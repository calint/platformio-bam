#pragma once
// abstraction of the device needed by 'main.cpp'

class device {
public:
  virtual ~device() = default;
  virtual void init() = 0;
  virtual auto is_display_touched() -> bool = 0;
  virtual void get_display_touch(uint16_t &x, uint16_t &y,
                                 uint8_t &pressure) = 0;
  virtual auto asyncDMAIsBusy() -> bool = 0;
  virtual void asyncDMAWaitForCompletion() = 0;
  virtual void asyncDMAWriteBytes(uint8_t *data, uint32_t len) = 0;
};