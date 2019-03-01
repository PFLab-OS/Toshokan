#pragma once
#include <assert.h>
#include <stdint.h>
#include "result.h"

// TODO: move this to another file
#define PAGE_SIZE 4096

/*
DOC START
# channel
Channel is a communication method between hakase and friend.
!!! Do not call Channel class directly. Use ChannelAccessor instead. !!!

when you use channel,
 - each id must be unique in a system. Typically, you can set processor core id
   as an id.
 - do not expect high performance.(on both latency and throughput)
   If it's needed, create another communication mechanism(e.g. shared memory).
 - send all data through channel. You had better not use pointer to pass data.
   Hakase and friend have different virtual memory space. It is very difficult
to access from one to the other.
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
    static constexpr Id kNull() { return Id(-1); }

   private:
    int32_t _val;
  } __attribute__((packed));
  class Signal {
   public:
    constexpr Signal() : _val(0) {}
    explicit constexpr Signal(const int32_t val) : _val(val){};
    Signal(const Signal &obj) = default;
    Signal &operator=(const Signal &obj) = default;
    bool operator==(const Signal &obj) { return obj._val == _val; }
    bool operator!=(const Signal &obj) { return obj._val != _val; }
    int32_t GetVal() { return _val; }
    
    static constexpr Signal kNull() {
      return Signal(0);
    }
    static constexpr Signal kCallback() {
      return Signal(1);
    }
    static constexpr Signal kPrint() {
      return Signal(2);
    }
    static constexpr Signal kExec() {
      return Signal(3);
    }
    static constexpr Signal kRwMemory() {
      return Signal(4);
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
    const Id null = Id::kNull();
    if (Id::CompareAndSwap(&_header->src_id, null, _my_id)) {
      _header->type = Signal::kNull();
      return Result<bool>(true);
    } else {
      return Result<bool>();
    }
  }
  void SendSignal(Id dest, Signal type) {
    assert(_header->src_id == _my_id);
    assert(_header->dest_id == Id::kNull());
    assert(_header->type == Signal::kNull());
    assert(type != Signal::kNull());
    _header->dest_id = dest;
    _header->type = type;
  }
  Result<int32_t> CheckIfReturned() {
    assert(_header->src_id == _my_id);
    if (_header->type == Signal::kNull()) {
      return Result<int32_t>(_header->rval);
    } else {
      return Result<int32_t>();
    }
  }
  void Release() {
    assert(_header->src_id == _my_id);
    assert(_header->type == Signal::kNull());
    _header->dest_id = Id::kNull();
    _header->src_id = Id::kNull();
  }
  bool IsSignalArrived() {
    return (_header->dest_id == _my_id && _header->type != Signal::kNull());
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
    _header->type = Signal::kNull();
  }
  static void InitBuffer(uint8_t *address) {
    Header *header = reinterpret_cast<Header *>(address);
    header->src_id = Id::kNull();
    header->dest_id = Id::kNull();
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

#include <_memory.h>
#if defined(__HAKASE__)

class H2F2 : public Channel2 {
 public:
  H2F2() = delete;
  H2F2(char *address)
      : Channel2(reinterpret_cast<uint8_t *>(address), Channel2::Id(0)) {}
};

class F2H2 : public Channel2 {
 public:
  F2H2() = delete;
  F2H2(char *address)
      : Channel2(reinterpret_cast<uint8_t *>(address), Channel2::Id(0)) {}
};

class I2H2 : public Channel2 {
 public:
  I2H2() = delete;
  I2H2(char *address)
      : Channel2(reinterpret_cast<uint8_t *>(address), Channel2::Id(0)) {}
};

#elif defined(__FRIEND__)

class H2F2 : public Channel2 {
 public:
  H2F2() : Channel2(reinterpret_cast<uint8_t *>(MemoryMap::kH2f), GetId()) {}

 private:
  static Channel2::Id GetId() {
    int32_t my_id;
    asm volatile("movl %%fs:0x4, %0" : "=r"(my_id));
    return Channel2::Id(my_id);
  }
};

class F2H2 : public Channel2 {
 public:
  F2H2() : Channel2(reinterpret_cast<uint8_t *>(MemoryMap::kF2h), GetId()) {}

 private:
  static Channel2::Id GetId() {
    int32_t my_id;
    asm volatile("movl %%fs:0x4, %0" : "=r"(my_id));
    return Channel2::Id(my_id);
  }
};

class I2H2 : public Channel2 {
 public:
  I2H2() : Channel2(reinterpret_cast<uint8_t *>(MemoryMap::kI2h), GetId()) {}

 private:
  static Channel2::Id GetId() {
    int32_t my_id;
    asm volatile("movl %%fs:0x4, %0" : "=r"(my_id));
    return Channel2::Id(my_id);
  }
};

#endif /* defined(__HAKASE__) */
