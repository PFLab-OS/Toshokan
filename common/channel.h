#pragma once
#include <stdint.h>
// DEBUG

class Channel {
public:
  int32_t SendSignal(int16_t type);
  void Reserve(int16_t id);
  void Write(int offset, uint8_t data);
  uint8_t Read(int offset);
  void Release();
private:
};
