#pragma once
#include <assert.h>
#include "type.h"
#include "result.h"

/*
 * Channel : communication channel between hakase and friend
 * !!! Do not call Channel class directly. Use ChannelAccessor instead. !!!
 *
 * Each Id must be unique in a system.
 */
class Channel2 {
public:
  class Id {
  public:
    Id() = delete;
    Id(int32_t val) : _val(val) {
    }
    Id(const Id &id) : _val(id._val) {
    }
    bool operator==(const Id &id) {
      return id._val == _val;
    }
    bool operator!=(const Id &id) {
      return id._val != _val;
    }
    Id &operator=(const Id &id) {
      _val = id._val;
      return *this;
    }
    static bool CompareAndSwap(Id *ptr, const Id &oldval, const Id &newval) {
      return __sync_bool_compare_and_swap(&ptr->_val, oldval._val, newval._val);
    }
    static Id Null() {
      return Id(-1);
    }
  private:
    int32_t _val;
  } __attribute__((packed));
  
  Channel2() = delete;
  Channel2(uint8_t *address, Id my_id) : _address(address), _header(reinterpret_cast<Header *>(address)), _my_id(my_id) {
  }
  Result<bool> Reserve() {
    const Id null = Id::Null();
    if (Id::CompareAndSwap(&_header->src_id, null, _my_id)) {
      _header->type = 0;
      return Result<bool>(true);
    } else {
      return Result<bool>();
    }
  }
  void SendSignal(Id dest, int32_t type) {
    assert(type != 0);
    assert(_header->src_id == _my_id);
    assert(_header->dest_id == Id::Null());
    assert(_header->type == 0);
    _header->dest_id = dest;
    _header->type = type;
  }
  Result<int32_t> CheckIfReturned() {
    asm volatile("":::"memory");
    assert(_header->src_id == _my_id);
    if (_header->type == 0) {
      return Result<int32_t>(_header->rval);
    } else {
      return Result<int32_t>();
    }
  }
  void Release() {
    assert(_header->src_id == _my_id);
    assert(_header->type == 0);
    _header->dest_id = Id::Null();
    _header->src_id = Id::Null();
  }
  Result<int32_t> CheckIfNewSignalArrived() {
    asm volatile("":::"memory");
    if (_header->dest_id == _my_id && _header->type != 0) {
      return Result<int32_t>(_header->type);
    } else {
      return Result<int32_t>();
    }
  }
  void Return(int32_t rval) {
    _header->rval = rval;
    _header->type = 0;
  }
  static void InitBuffer(uint8_t *address) {
    Header *header = reinterpret_cast<Header *>(address);
    header->src_id = Id::Null();
    header->dest_id = Id::Null();
  }
  void Write(int offset, uint8_t data) {
    _address[sizeof(Header) + offset] = data;
  }
  uint8_t Read(int offset) {
    return _address[sizeof(Header) + offset];
  }
  static const int kBufAddress = 4096;
private:
  struct Header {
    int32_t type;
    Id src_id;
    Id dest_id;
    int32_t rval;
  } __attribute__((packed));
  static const int kHeaderSize = sizeof(Channel2::Header);
  uint8_t * const _address;
  Header * const _header;
  const Id _my_id;
public:
  static const int kDataSize = kBufAddress - sizeof(Header);
};

