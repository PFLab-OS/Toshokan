#pragma once
#include <string.h>
#include <stdint.h>
#include "common/result.h"
#include "common/channel.h"

namespace MemoryAccessor {
  static const size_t kTransferSize = 2048;
  
  class MemoryAccessorBase {
  protected:
    static const size_t kTransferDataOffset = 2040;
    enum class Signature : uint32_t {
      kRead = 0,
      kWrite = 1,
    };
  };
  
  class Writer : public MemoryAccessorBase {
  public:
    Writer(Channel &ch, const uint64_t address, void *buf, size_t size) : _ch(ch), _address(address), _buf(reinterpret_cast<uint8_t *>(buf)), _size(size) {
    }
    Writer() = delete;
    
    Result<bool> Do() {
      for(size_t offset = 0; offset < _size; offset += kTransferSize) {
        size_t size = (_size - offset) > kTransferSize ? kTransferSize : (_size - offset);

        Channel::Accessor ch_ac(_ch, 4);
        ch_ac.Write(0, static_cast<uint32_t>(Signature::kWrite));
        ch_ac.Write(8, _address + offset);
        ch_ac.Write(16, size);

        for(size_t i = 0; i < size; i++) {
          ch_ac.Write(kTransferDataOffset + i, _buf[offset + i]);
        }
        if (ch_ac.Do() != 0) {
          return Result<bool>();
        }
      }
      return Result<bool>(true);
    }
  private:
    static const uint32_t kSignatureWrite = 1;
    Channel &_ch;
    const uint64_t _address;
    uint8_t *_buf;
    size_t _size;
  };
  
  class Reader : public MemoryAccessorBase {
  public:
    Reader(Channel &ch, const uint64_t address, void *buf, size_t size) : _ch(ch), _address(address), _buf(reinterpret_cast<uint8_t *>(buf)), _size(size) {
    }
    Reader() = delete;
    
    Result<bool> Do() {
      for(size_t offset = 0; offset < _size; offset += kTransferSize) {
        size_t size = (_size - offset) > kTransferSize ? kTransferSize : (_size - offset);

        Channel::Accessor ch_ac(_ch, 4);
        ch_ac.Write(0, static_cast<uint32_t>(Signature::kRead));
        ch_ac.Write(8, _address + offset);
        ch_ac.Write(16, size);

        if (ch_ac.Do() != 0) {
          return Result<bool>();
        }
        for(size_t i = 0; i < size; i++) {
          uint8_t data;
          ch_ac.Read(kTransferDataOffset + i, data);
          _buf[offset + i] = data;
        }
      }
      return Result<bool>(true);
    }
  private:
    Channel &_ch;
    const uint64_t _address;
    uint8_t *_buf;
    size_t _size;
  };
}
