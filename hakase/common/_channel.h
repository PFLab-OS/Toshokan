#pragma once

#include "common/type.h"
#include "memory.h"

class Channel {
public:
  // void Read(int offset, uint8_t &data) { ReadSub(offset, data); }
  // void Write(int offset, uint8_t data) { WriteSub(offset, data); }
  // void Read(int offset, uint16_t &data) { ReadSub(offset, data); }
  // void Write(int offset, uint16_t data) { WriteSub(offset, data); }
  // void Read(int offset, uint32_t &data) { ReadSub(offset, data); }
  // void Write(int offset, uint32_t data) { WriteSub(offset, data); }
  // void Read(int offset, uint64_t &data) { ReadSub(offset, data); }
  // void Write(int offset, uint64_t data) { WriteSub(offset, data); }
  // template <class T>
  // T *GetRawPtr() {
  //   return reinterpret_cast<T *>(_address);
  // }
  // void WriteString(const char *str) {
  //   while (true) {
  //     Reserve();
  //     Write(0, static_cast<uint8_t>(*str));
  //     SendSignal(2);
  //     Release();

  //     if (*str == '\0') {
  //       break;
  //     }
      
  //     str++;
  //   }
  // }
  // void WaitNewSignal(int16_t &type) {
  //   while(true) {
  //     SignalTypeAndIdContainer c = GetSignalTypeAndId();
  //     if (c.id == _my_id && c.type != 0) {
  //       type = c.type;
  //       return;
  //     }
  //     asm volatile("pause" ::: "memory");
  //   }
  // }
  // void Reserve() {
  //   while(true) {
  //     SignalTypeAndId u;
  //     u.c.id = _my_id;
  //     u.c.type = 0;
  //     if (__sync_bool_compare_and_swap(&reinterpret_cast<SignalTypeAndId *>(&_address[0])->u32, 0, u.u32)) {
  //       break;
  //     }
  //     asm volatile("pause" ::: "memory");
  //   }
  //   for (unsigned int i = 8 / sizeof(uint64_t); i < 4096 / sizeof(uint64_t); i++) {
  //     reinterpret_cast<uint64_t *>(_address)[i] = 0;
  //   }
  // }
  // void Release() {
  //   reinterpret_cast<SignalTypeAndId *>(&_address[0])->u32 = 0;
  // }
  // int SendSignal(int16_t type) {
  //   if (type == 0) {
  //     return 0;
  //   }
  //   SetSignalType(type);

  //   while(true) {
  //     SignalTypeAndIdContainer c = GetSignalTypeAndId();
  //     if (c.type == 0) {
  //       break;
  //     }
  //     asm volatile("pause" ::: "memory");
  //   }

  //   return GetReturnValueRef();
  // }
  // void Return(int32_t rval) {
  //   GetReturnValueRef() = rval;
  //   SetSignalType(0);
  // }
protected:
  // Channel() {
  //   asm volatile("movw %%fs:0x0, %0" : "=r"(_my_id));
  // }
  // char *_address;
private:
  // template<class T>
  //   void ReadSub(int offset, T &data) {
  //   data = GetRawPtr<T>()[(offset + 8) / sizeof(T)];
  // }
  // template<class T>
  //   void WriteSub(int offset, T data) {
  //   GetRawPtr<T>()[(offset + 8) / sizeof(T)] = data;
  // }
  // struct SignalTypeAndIdContainer {
  //   int16_t type;
  //   int16_t id;
  // } __attribute__((packed));
  // static_assert(sizeof(SignalTypeAndIdContainer) == sizeof(uint32_t), "");
  // union SignalTypeAndId {
  //   uint32_t u32;
  //   SignalTypeAndIdContainer c;
  // };
  // static_assert(sizeof(SignalTypeAndId) == sizeof(uint32_t), "");
  // SignalTypeAndIdContainer GetSignalTypeAndId() {
  //   return reinterpret_cast<SignalTypeAndId *>(&_address[0])->c;
  // }
  // void SetSignalType(int16_t type) {
  //   if (reinterpret_cast<SignalTypeAndId *>(&_address[0])->c.id == 0) {
  //     // panic
  //     while(true) {
  //       __asm__ volatile("cli;hlt;");
  //     }
  //   }
  //   reinterpret_cast<SignalTypeAndId *>(&_address[0])->c.type = type;
  // }
  // int32_t &GetReturnValueRef() {
  //   return reinterpret_cast<int32_t *>(_address)[1];
  // }
  // int16_t _my_id;
};

// class H2F : public Channel {
// public:
//   H2F() { _address = reinterpret_cast<char *>(MemoryMap::kH2f); }
// };

// class F2H : public Channel {
// public:
//   F2H() { _address = reinterpret_cast<char *>(MemoryMap::kF2h); }
// };
