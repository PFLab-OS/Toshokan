#pragma once
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "channel_accessor2.h"
#include "result.h"

namespace MemoryAccessor {
static const size_t kTransferSize = 2048;

class MemoryAccessorBase {
 protected:
  static const size_t kTransferDataOffset = 1024;
  enum class Signature : uint32_t {
    kRead = 0,
    kWrite = 1,
  };
};

class Writer : public MemoryAccessorBase {
 public:
 Writer(Channel2 &ch, Channel2::Id id, const uint64_t address, void *buf,
         size_t size)
      : _ch(ch),
        _id(id),
        _address(address),
        _buf(reinterpret_cast<uint8_t *>(buf)),
        _size(size) {}
  Writer() = delete;

  Result<bool> Do() {
    for (size_t offset = 0; offset < _size; offset += kTransferSize) {
      size_t size =
          (_size - offset) > kTransferSize ? kTransferSize : (_size - offset);

      CallerChannelAccessor caller_ca(_ch, _id, Channel2::Signal::kRwMemory());
      
      caller_ca.Write<uint32_t>(CallerChannelAccessor::Offset<uint32_t>(0), static_cast<uint32_t>(Signature::kWrite));
      caller_ca.Write<size_t>(CallerChannelAccessor::Offset<size_t>(8), _address + offset);
      caller_ca.Write<size_t>(CallerChannelAccessor::Offset<size_t>(16), size);

      for (size_t i = 0; i < size; i++) {
        caller_ca.Write<uint8_t>(CallerChannelAccessor::Offset<uint8_t>(kTransferDataOffset + i), _buf[offset + i]);
      }
      if (caller_ca.Call() != 0) {
        return Result<bool>();
      }
    }
    return Result<bool>(true);
  }

 private:
  static const uint32_t kSignatureWrite = 1;
  Channel2 &_ch;
  Channel2::Id _id;
  const uint64_t _address;
  uint8_t *_buf;
  size_t _size;
};

class Reader : public MemoryAccessorBase {
 public:
 Reader(Channel2 &ch, Channel2::Id id, const uint64_t address, void *buf,
         size_t size)
      : _ch(ch),
        _id(id),
        _address(address),
        _buf(reinterpret_cast<uint8_t *>(buf)),
        _size(size) {}
  Reader() = delete;

  Result<bool> Do() {
    for (size_t offset = 0; offset < _size; offset += kTransferSize) {
      size_t size =
          (_size - offset) > kTransferSize ? kTransferSize : (_size - offset);

      CallerChannelAccessor caller_ca(_ch, _id, Channel2::Signal::kRwMemory());
      caller_ca.Write<uint32_t>(CallerChannelAccessor::Offset<uint32_t>(0), static_cast<uint32_t>(Signature::kRead));
      caller_ca.Write<size_t>(CallerChannelAccessor::Offset<size_t>(8), _address + offset);
      caller_ca.Write<size_t>(CallerChannelAccessor::Offset<size_t>(16), size);

      if (caller_ca.Call() != 0) {
        return Result<bool>();
      }
      for (size_t i = 0; i < size; i++) {
        _buf[offset + i] = caller_ca.Read<uint8_t>(CallerChannelAccessor::Offset<uint8_t>(kTransferDataOffset + i));
      }
    }
    return Result<bool>(true);
  }

 private:
  Channel2 &_ch;
  Channel2::Id _id;
  const uint64_t _address;
  uint8_t *_buf;
  size_t _size;
};
}  // namespace MemoryAccessor
