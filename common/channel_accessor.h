#pragma once
#include <assert.h>
#include "channel/hakase.h"
#include "type.h"

template <int kBufSize = Channel::kDataAreaSizeMax>
class ChannelAccessor {
 public:
  ChannelAccessor(Channel &ch, int16_t type) : _ch(ch), _type(type) {
    for (size_t i = 0; i < sizeof(_buffer) / sizeof(_buffer[0]); i++) {
      _buffer[i] = 0;
    }
  }
  ChannelAccessor() = delete;
  template <class T>
  void Write(int offset, T data) {
    assert(!_signal_sended);
    assert((offset % sizeof(T)) == 0);
    assert(offset + sizeof(T) <= kBufSize);
    reinterpret_cast<T *>(_buffer)[offset / sizeof(T)] = data;
  }
  template <class T>
  T Read(int offset) {
    assert(_signal_sended);
    assert((offset % sizeof(T)) == 0);
    assert(offset + sizeof(T) <= Channel::kDataAreaSizeMax);
    return reinterpret_cast<T *>(_buffer)[offset / sizeof(T)];
  }
  int32_t Do(int16_t id) {
    _ch.Reserve(id);
    for (int i = 0; i < kBufSize; i++) {
      _ch.Write(i, _buffer[i]);
    }
    int32_t rval = _ch.SendSignal(_type);
    _signal_sended = true;
    for (int i = 0; i < kBufSize; i++) {
      _buffer[i] = _ch.Read(i);
    }
    _ch.Release();
    return rval;
  }

 private:
  static_assert(kBufSize <= Channel::kDataAreaSizeMax, "");
  Channel &_ch;
  const int16_t _type;
  uint8_t _buffer[kBufSize];
  bool _signal_sended = false;
};
