#pragma once

#include "type.h"

class Channel {
public:
  int32_t GetType() { return *((int32_t *)_address); }
  void SetType(int32_t type) { *((int32_t *)_address) = type; }
  void Read(int offset, uint8_t &data) { data = _address[offset + 4]; }
  void Write(int offset, uint8_t data) { _address[offset + 4] = data; }
  void WriteString(const char *str) {
    while (true) {
      while (GetType() != 0) {
        asm volatile("pause" ::: "memory");
      }

      Write(0, *str);

      SetType(2);

      if (*str == '\0') {
        break;
      }

      str++;
    }
  }
  void WaitNewSignal() {
    while (GetType() == 0) {
      asm volatile("pause" ::: "memory");
    }
  }
  int SendSignal(int32_t type) {
    int rval;
    if (type == 0) {
      return 0;
    }
    SetType(type);

    while ((rval = GetType()) != type) {
      asm volatile("pause" ::: "memory");
    }
    return rval;
  }

protected:
  Channel() {}
  char *_address;
};

class H2F : public Channel {
public:
  H2F() { _address = (char *)0x2000; }
};

class F2H : public Channel {
public:
  F2H() { _address = (char *)0x3000; }
};
