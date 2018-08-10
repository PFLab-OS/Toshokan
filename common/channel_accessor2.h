#pragma once
#include "channel2.h"
#include <assert.h>
#include "type.h"

template<int kBufSize>
class CallerChannelAccessor {
public:
  CallerChannelAccessor() = delete;
  CallerChannelAccessor(Channel2 &ch, Channel2::Id dest, Channel2::Signal signal) : _dest(dest), _signal(signal), _ch(ch) {
    for(size_t i = 0; i < sizeof(_buffer) / sizeof(_buffer[0]); i++) {
      _buffer[i] = 0;
    }
  }
  // for debugging
  virtual void Do() {
  }
  template<class T>
  class Offset {
  public:
    Offset() = delete;
    explicit Offset(int offset) : _offset(offset) {
      assert((offset % sizeof(T)) == 0);
      assert(offset + sizeof(T) <= kBufSize);
    }
    const int GetValue() {
      return _offset;
    }
  private:
    const int _offset;
  };
  template<class T>
  void Write(Offset<T> offset, T data) {
    assert(!_signal_sended);
    reinterpret_cast<T *>(_buffer)[offset.GetValue() / sizeof(T)] = data;
  }
  template<class T>
  T Read(Offset<T> offset) {
    assert(_signal_sended);
    return reinterpret_cast<T *>(_buffer)[offset.GetValue() / sizeof(T)];
  }
  int32_t Call() {
    while(_ch.Reserve().IsError()) {
    }
    for(int i = 0; i < kBufSize; i++) {
      _ch.Write(i, _buffer[i]);
    }
    _ch.SendSignal(_dest, _signal);
    int32_t rval;
    while(true) {
      auto r = _ch.CheckIfReturned();
      if (r.IsError()) {
        Do();
      } else {
        rval = r.Unwrap();
        break;
      }
    }
    _signal_sended = true;
    for(int i = 0; i < kBufSize; i++) {
      _buffer[i] = _ch.Read(i);
    }
    _ch.Release();
    return rval;
  }
private:
  Channel2::Id _dest;
  Channel2::Signal _signal;
  Channel2 &_ch;
  static_assert(kBufSize <= Channel2::kDataSize, "");
  uint8_t _buffer[kBufSize];
  bool _signal_sended = false;
};

template<int kBufSize>
class CalleeChannelAccessor {
public:
  CalleeChannelAccessor(Channel2 &ch) : signal(1) /* dummy signal */ {
  }
  Channel2::Signal GetSignal() {
  }
private:
  Channel2::Signal signal;
};

