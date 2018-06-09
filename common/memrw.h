#pragma once
#include <string.h>
#include <stdint.h>
#include "result.h"
#include "channel.h"

namespace MemoryAccessor {
  static const size_t kTransferSize = 2048;
  
  class MemoryAccessorBase {
  protected:
    static const size_t kTransferDataOffset = 2048;
  };
  
  class Writer : public MemoryAccessorBase {
  public:
    Writer(Channel &ch, const uint64_t address, void *buf, size_t size) : _ch(ch), _address(address), _buf(reinterpret_cast<uint8_t *>(buf)), _size(size) {
    }
    Writer() = delete;
    
    Result<bool> Do() {
      for(size_t offset = 0; offset < _size; offset += kTransferSize) {
        size_t size = (_size - offset) > kTransferSize ? kTransferSize : (_size - offset);
        
        _ch.Reserve();
        _ch.Write(0, kSignatureWrite);
        _ch.Write(8, _address + offset);
        _ch.Write(16, size);
        
        uint8_t *ch_buf = _ch.GetRawPtr<uint8_t>() + kTransferDataOffset;
        memcpy(ch_buf, _buf + offset, size);
        if (_ch.SendSignal(4) != 0) {
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
}
