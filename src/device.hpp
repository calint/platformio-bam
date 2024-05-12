#pragma once

class device {
public:
  virtual ~device() = default;
  virtual void init() = 0;
  virtual auto is_display_touched() -> bool = 0;
  virtual void get_display_touch(int16_t &x, int16_t &y, int16_t &z) = 0;
  virtual auto asyncDMAIsBusy() -> bool = 0;
  virtual void asyncDMAWaitForCompletion() = 0;
  virtual void asyncDMAWriteBytes(uint8_t *data, uint32_t len) = 0;
};