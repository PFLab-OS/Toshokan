#pragma once
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "channel2.h"

/*
DOC START
# channel/ChannelAccessor
ChannelAccessor class: Wrapper of Channel

CallerChannelAccessor can be used by Caller and CalleeChannelAccessor can be
used by Callee DOC END
*/

class CallerChannelAccessor {
 public:
  CallerChannelAccessor() = delete;
  CallerChannelAccessor(Channel2 &ch, Channel2::Id dest,
                        Channel2::Signal signal)
      : _dest(dest), _signal(signal), _ch(ch) {
    for (size_t i = 0; i < sizeof(_buffer) / sizeof(_buffer[0]); i++) {
      _buffer[i] = 0;
    }
  }
#ifdef __CPPUTEST__
  // for debugging
  virtual void Do() {}
#endif /* __CPPUTEST__ */
  template <class T>
  class Offset {
   public:
    Offset() = delete;
    explicit Offset(int offset) : _offset(offset) {
      assert((offset % sizeof(T)) == 0);
      assert(offset + sizeof(T) <= Channel2::kDataSize);
    }
    const int GetValue() { return _offset; }

   private:
    const int _offset;
  };
  template <class T>
  void Write(Offset<T> offset, T data) {
    assert(!_signal_sended);
    reinterpret_cast<T *>(_buffer)[offset.GetValue() / sizeof(T)] = data;
  }
  template <class T>
  T Read(Offset<T> offset) {
    assert(_signal_sended);
    return reinterpret_cast<T *>(_buffer)[offset.GetValue() / sizeof(T)];
  }
  int32_t Call() {
    while (_ch.Reserve().IsError()) {
      asm volatile("pause" ::: "memory");
    }
    for (int i = 0; i < Channel2::kDataSize; i++) {
      _ch.Write(i, _buffer[i]);
    }
    _ch.SendSignal(_dest, _signal);
    int32_t rval;
    while (true) {
      auto r = _ch.CheckIfReturned();
      if (r.IsError()) {
#ifdef __CPPUTEST__
        // for debugging
        Do();
#endif /* __CPPUTEST__ */
      } else {
        rval = r.Unwrap();
        break;
      }
      asm volatile("pause" ::: "memory");
    }
    _signal_sended = true;
    for (int i = 0; i < Channel2::kDataSize; i++) {
      _buffer[i] = _ch.Read(i);
    }
    _ch.Release();
    return rval;
  }

 private:
  Channel2::Id _dest;
  Channel2::Signal _signal;
  Channel2 &_ch;
  uint8_t _buffer[Channel2::kDataSize];
  bool _signal_sended = false;
};

/*
 * CalleeChannelAccessor
 * !!! An instance of CalleeChannelAccessor must be unique per thread. !!!
 */
class CalleeChannelAccessor {
 public:
  CalleeChannelAccessor(Channel2 &ch) : _ch(ch), signal(1) /* dummy signal */ {}
  void ReceiveSignal() {
    assert(!_accessible);
    while (!_ch.IsSignalArrived()) {
      asm volatile("pause" ::: "memory");
    }
    _accessible = true;
  }
  template <class T>
  class Offset {
   public:
    Offset() = delete;
    explicit Offset(int offset) : _offset(offset) {
      assert((offset % sizeof(T)) == 0);
      assert(offset + sizeof(T) <= Channel2::kDataSize);
    }
    const int GetValue() { return _offset; }

   private:
    const int _offset;
  };
  template <class T>
  void Write(Offset<T> offset, T data) {
    assert(_accessible);
    uint8_t *buf = reinterpret_cast<uint8_t *>(&data);
    for (int i = 0; i < sizeof(T); i++) {
      _ch.Write(offset.GetValue() + i, buf[i]);
    }
  }
  template <class T>
  T Read(Offset<T> offset) {
    assert(_accessible);
    uint8_t buf[sizeof(T)];
    for (int i = 0; i < sizeof(T); i++) {
      buf[i] = _ch.Read(offset.GetValue() + i);
    }
    return *(reinterpret_cast<T *>(buf));
  }
  Channel2::Signal GetSignal() {
    assert(_accessible);
    return _ch.GetArrivedSignal();
  }
  Channel2::Id GetId() {
    assert(_accessible);
    return _ch.GetSenderId();
  }
  void Return(int32_t rval) {
    assert(_accessible);
    _ch.Return(rval);
    _accessible = false;
  }

 private:
  Channel2 &_ch;
  Channel2::Signal signal;
  bool _accessible = false;
};
