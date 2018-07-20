#pragma once
#include <assert.h>
#include "common/_memory.h"
#include "panic.h"
#include "type.h"

#define PAGE_SIZE 4096

/*
 * Channel : communication channel between hakase and friend
 * !!! Do not call Channel class directly. Use ChannelAccessor instead. !!!
 */
class Channel {
 public:
  int32_t SendSignal(int16_t type);
  void Reserve(int16_t id);
  // return value: core id
  int WaitNewSignal(int16_t &type);
  void Write(int offset, uint8_t data);
  // deprecated
  void WriteString(int16_t id, const char *str) {
    while (true) {
      Reserve(id);
      Write(0, static_cast<uint8_t>(*str));
      SendSignal(2);
      Release();

      if (*str == '\0') {
        break;
      }

      str++;
    }
  }
  uint8_t Read(int offset);
  // deprecated
  template <class T>
  T OldRead(int offset) {
    return reinterpret_cast<T *>(_address)[(offset + 8) / sizeof(T)];
  }
  void Release();
  void Return(int32_t rval);
  static const int kDataAreaSizeMax = 4096 - 8;

 protected:
  Channel();
  uint8_t *_address;

 private:
  struct SignalTypeAndIdContainer {
    int16_t type;
    int16_t id;
  } __attribute__((packed));

  static_assert(sizeof(SignalTypeAndIdContainer) == sizeof(uint32_t), "");

  union SignalTypeAndId {
    uint32_t u32;
    SignalTypeAndIdContainer c;
  };

  void SetSignalType(int16_t type) {
    if (reinterpret_cast<SignalTypeAndId *>(&_address[0])->c.id == 0) {
      panic();
    }
    reinterpret_cast<SignalTypeAndId *>(&_address[0])->c.type = type;
  }
  int32_t &GetReturnValueRef() {
    return reinterpret_cast<int32_t *>(_address)[1];
  }
  bool IsMyId(int16_t id);
  int16_t GetCalleeId(int16_t id);
  SignalTypeAndIdContainer GetSignalTypeAndId() {
    return reinterpret_cast<SignalTypeAndId *>(&_address[0])->c;
  }
  int16_t _my_id;
};

#if defined(__HAKASE__) || defined(__FRIEND__)
inline int Channel::WaitNewSignal(int16_t &type) {
  while (true) {
    SignalTypeAndIdContainer c = GetSignalTypeAndId();
    if (IsMyId(c.id) && c.type != 0) {
      type = c.type;
      return c.id;
    }
    asm volatile("pause" ::: "memory");
  }
}

inline void Channel::Reserve(int16_t id) {
  while (true) {
    SignalTypeAndId u;
    u.c.id = GetCalleeId(id);
    u.c.type = 0;
    if (__sync_bool_compare_and_swap(
            &reinterpret_cast<SignalTypeAndId *>(&_address[0])->u32, 0,
            u.u32)) {
      break;
    }
    asm volatile("pause" ::: "memory");
  }
  for (unsigned int i = 8 / sizeof(uint64_t); i < 4096 / sizeof(uint64_t);
       i++) {
    reinterpret_cast<uint64_t *>(_address)[i] = 0;
  }
}

inline void Channel::Write(int offset, uint8_t data) {
  _address[offset + 8] = data;
}

inline uint8_t Channel::Read(int offset) { return _address[offset + 8]; }

inline int Channel::SendSignal(int16_t type) {
  if (type == 0) {
    return 0;
  }
  SetSignalType(type);

  while (true) {
    SignalTypeAndIdContainer c = GetSignalTypeAndId();
    if (c.type == 0) {
      break;
    }
    asm volatile("pause" ::: "memory");
  }

  return GetReturnValueRef();
}

inline void Channel::Release() {
  reinterpret_cast<SignalTypeAndId *>(&_address[0])->u32 = 0;
}

inline void Channel::Return(int32_t rval) {
  GetReturnValueRef() = rval;
  SetSignalType(0);
}

#endif

#if defined(__HAKASE__)

class H2F : public Channel {
 public:
  H2F() = delete;
  H2F(char *address) { _address = reinterpret_cast<uint8_t *>(address); }
};

class F2H : public Channel {
 public:
  F2H() = delete;
  F2H(char *address) { _address = reinterpret_cast<uint8_t *>(address); }
};

class I2H : public Channel {
 public:
  I2H() = delete;
  I2H(char *address) { _address = reinterpret_cast<uint8_t *>(address); }
};

inline Channel::Channel() { _my_id = 0; }

inline bool Channel::IsMyId(int16_t id) { return id != _my_id; }

inline int16_t Channel::GetCalleeId(int16_t id) { return id; }

#elif defined(__FRIEND__)

class H2F : public Channel {
 public:
  H2F() { _address = reinterpret_cast<uint8_t *>(MemoryMap::kH2f); }
};

class F2H : public Channel {
 public:
  F2H() { _address = reinterpret_cast<uint8_t *>(MemoryMap::kF2h); }
};

class I2H : public Channel {
 public:
  I2H() { _address = reinterpret_cast<uint8_t *>(MemoryMap::kI2h); }
};

inline Channel::Channel() { asm volatile("movw %%fs:0x0, %0" : "=r"(_my_id)); }

inline bool Channel::IsMyId(int16_t id) { return id == _my_id; }

inline int16_t Channel::GetCalleeId(int16_t id) {
  assert(id == 0);
  return _my_id;
}

#endif /* defined(__HAKASE__) */
