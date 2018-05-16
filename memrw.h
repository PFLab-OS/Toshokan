#pragma once
#include <string.h>
#include <stdint.h>
#include "channel.h"

class MemoryWriter {
 public:
  MemoryWriter(Channel &ch, const uint64_t address) : _ch(ch), _address(address) {
    _ch.Clear();
    ch.Write(0, kSignatureWrite);
    ch.Write(8, address);
  }
  MemoryWriter() = delete;
  void Copy(void *buf, int size) {
    _size = size;
    uint8_t *ch_buf = _ch.GetRawPtr<uint8_t>() + 2048 / sizeof(uint8_t);
    memcpy(ch_buf, buf, _size);
  }
  void Do() {
    assert(_ch.SendSignal(4) == 0);
  }
 private:
  static const uint32_t kSignatureWrite = 1;
  Channel &_ch;
  int _size = 0;
  const uint64_t _address;
};
