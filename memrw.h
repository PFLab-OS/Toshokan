#pragma once
#include <string.h>
#include <stdint.h>
#include "result.h"
#include "channel.h"

namespace MemoryAccessor {
  class DataSize {
  public:
    static Result<DataSize> Create(const int val) {
      if (val > 2048) {
        return Result<DataSize>();
      } else {
        return Result<DataSize>(DataSize(val));
      }
    }
    const int Get() const {
      return _val;
    }
  private:
    friend class Result<DataSize>;
    DataSize(const int val) : _val(val) {
      assert(_val <= 2048);
    }
    DataSize() : _val(0) {
    }
    const int _val;
  };

  class Writer {
  public:
    Writer(Channel &ch, const uint64_t address, const DataSize size) : _ch(ch), _address(address), _size(size) {
      _ch.Clear();
      _ch.Write(0, kSignatureWrite);
      _ch.Write(8, address);
    }
    Writer() = delete;
    
    Result<bool> Copy(void *buf) {
      if (_copied) {
        return Result<bool>();
      }
      uint8_t *ch_buf = _ch.GetRawPtr<uint8_t>() + 2048 / sizeof(uint8_t);
      memcpy(ch_buf, buf, _size.Get());
      _copied = true;
      return Result<bool>(true);
    }
    Result<bool> Do() {
      if (!_copied) {
        return Result<bool>();
      }
      assert(_ch.SendSignal(4) == 0);
      return Result<bool>(true);
    }
  private:
    
    static const uint32_t kSignatureWrite = 1;
    Channel &_ch;
    const uint64_t _address;
    const DataSize _size;
    bool _copied = false;
  };
}
