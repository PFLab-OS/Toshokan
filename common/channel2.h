#pragma once
#include <assert.h>
#include <stdint.h>
#include "result.h"

/*
 * Channel : communication channel between hakase and friend
 * !!! Do not call Channel class directly. Use ChannelAccessor instead. !!!
 *
 * when you use channel,
 *  - each id must be unique in a system. Typically, you can set processor core
 * id as an id.
 *  - do not expect high performance.(on both latency and throughput)
 *    If it's needed, create another communication mechanism(e.g. shared
 * memory).
 *  - send all data through channel. You had better not use pointer to pass
 * data. Hakase and friend have different virtual memory space. It is very
 * difficult to access from one to the other.
 */
class Channel2 {
 public:
  class Id {
   public:
    Id() = delete;
    explicit constexpr Id(const int32_t val) : _val(val) {}
    Id(const Id &id) = default;
    Id &operator=(const Id &id) = default;
    bool operator==(const Id &id) { return id._val == _val; }
    bool operator!=(const Id &id) { return id._val != _val; }
    static bool CompareAndSwap(Id *ptr, const Id &oldval, const Id &newval) {
      return __sync_bool_compare_and_swap(&ptr->_val, oldval._val, newval._val);
    }
    static constexpr Id Null() { return Id(-1); }

   private:
    int32_t _val;
  } __attribute__((packed));
  class Signal {
   public:
    constexpr Signal() : _val(0) {}
    explicit constexpr Signal(const int32_t val) : _val(val) {
    };
    Signal(const Signal &obj) = default;
    Signal &operator=(const Signal &obj) = default;
    bool operator==(const Signal &obj) { return obj._val == _val; }
    bool operator!=(const Signal &obj) { return obj._val != _val; }
    int32_t GetVal() { return _val; }
    
    static constexpr Signal Null() {
      return Signal(0);
    }

   private:
    int32_t _val;
  } __attribute__((packed));

  Channel2() = delete;
  Channel2(uint8_t *address, Id my_id)
      : _address(address),
        _header(reinterpret_cast<Header *>(address)),
        _my_id(my_id) {}
  Result<bool> Reserve() {
    const Id null = Id::Null();
    if (Id::CompareAndSwap(&_header->src_id, null, _my_id)) {
      _header->type = Signal::Null();
      return Result<bool>(true);
    } else {
      return Result<bool>();
    }
  }
  void SendSignal(Id dest, Signal type) {
    assert(_header->src_id == _my_id);
    assert(_header->dest_id == Id::Null());
    assert(_header->type == Signal::Null());
    assert(type != Signal::Null());
    _header->dest_id = dest;
    _header->type = type;
  }
  Result<int32_t> CheckIfReturned() {
    asm volatile("" ::: "memory");
    assert(_header->src_id == _my_id);
    if (_header->type == Signal::Null()) {
      return Result<int32_t>(_header->rval);
    } else {
      return Result<int32_t>();
    }
  }
  void Release() {
    assert(_header->src_id == _my_id);
    assert(_header->type == Signal::Null());
    _header->dest_id = Id::Null();
    _header->src_id = Id::Null();
  }
  bool IsSignalArrived() {
    asm volatile("" ::: "memory");
    return (_header->dest_id == _my_id && _header->type != Signal::Null());
  }
  Signal GetArrivedSignal() {
    assert(IsSignalArrived());
    return _header->type;
  }
  Id GetSenderId() {
    assert(IsSignalArrived());
    return _header->src_id;
  }
  void Return(int32_t rval) {
    assert(IsSignalArrived());
    _header->rval = rval;
    _header->type = Signal::Null();
  }
  static void InitBuffer(uint8_t *address) {
    Header *header = reinterpret_cast<Header *>(address);
    header->src_id = Id::Null();
    header->dest_id = Id::Null();
  }
  void Write(int offset, uint8_t data) {
    _address[sizeof(Header) + offset] = data;
  }
  uint8_t Read(int offset) { return _address[sizeof(Header) + offset]; }
  static const int kBufAddress = 4096;

 private:
  struct Header {
    Signal type;
    Id src_id;
    Id dest_id;
    int32_t rval;
  } __attribute__((packed));
  static const int kHeaderSize = sizeof(Channel2::Header);
  uint8_t *const _address;
  Header *const _header;
  const Id _my_id;

 public:
  static const int kDataSize = kBufAddress - sizeof(Header);
};

