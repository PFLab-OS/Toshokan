#pragma once
#include <stdint.h>
#include <stdlib.h>

#define PAGE_SIZE     4096

class Channel {
public:
  void Read(int offset, uint8_t &data) { ReadSub(offset, data); }
  void Write(int offset, uint8_t data) { WriteSub(offset, data); }
  void Read(int offset, uint16_t &data) { ReadSub(offset, data); }
  void Write(int offset, uint16_t data) { WriteSub(offset, data); }
  void Read(int offset, uint32_t &data) { ReadSub(offset, data); }
  void Write(int offset, uint32_t data) { WriteSub(offset, data); }
  void Read(int offset, uint64_t &data) { ReadSub(offset, data); }
  void Write(int offset, uint64_t data) { WriteSub(offset, data); }
  template <class T>
  T *GetRawPtr() {
    return reinterpret_cast<T *>(_address);
  }
  void Reserve() {
    if (_reserved) {
      fprintf(stderr, "Channel: error: already reserved\n");
      exit(255);
    }
    _reserved = true;
    for (unsigned int i = 0; i < 4096 / sizeof(uint64_t); i++) {
      reinterpret_cast<uint64_t *>(_address)[i] = 0;
    }
  }
  void WriteString(const char *str) {
    while (true) {
      Write(0, static_cast<uint8_t>(*str));

      SendSignal(2);
      
      if (*str == '\0') {
        break;
      }

      str++;
    }
  }
  int WaitNewSignal() {
    int type;
    while ((type = GetSignalTypeRef()) == 0) {
      asm volatile("pause" ::: "memory");
    }
    return type;
  }
  int SendSignal(int32_t type) {
    if (type == 0) {
      return 0;
    }
    if (!_reserved) {
      fprintf(stderr, "Channel: error: not reserved\n");
      exit(255);
    }
    
    GetSignalTypeRef() = type;

    while (GetSignalTypeRef() != 0) {
      asm volatile("pause" ::: "memory");
    }
    
    int rval = GetReturnValueRef();
    _reserved = false;
    return rval;
  }
  void Return(int32_t rval) {
    GetReturnValueRef() = rval;
    GetSignalTypeRef() = 0;
  }
protected:
  Channel() {
  }
  char *_address;
 private:
  template<class T>
    void ReadSub(int offset, T &data) {
    data = GetRawPtr<T>()[(offset + 8) / sizeof(T)];
  }
  template<class T>
    void WriteSub(int offset, T data) {
    GetRawPtr<T>()[(offset + 8) / sizeof(T)] = data;
  }
  int32_t &GetSignalTypeRef() {
    return reinterpret_cast<int32_t *>(_address)[0];
  }
  int32_t &GetReturnValueRef() {
    return reinterpret_cast<int32_t *>(_address)[1];
  }
  bool _reserved = false;
};

class H2F : public Channel {
public:
  H2F() = delete;
  H2F(char *address) {
    _address = address;
  }
};

class F2H : public Channel {
public:
  F2H() = delete;
  F2H(char *address) {
    _address = address;
  }
};
