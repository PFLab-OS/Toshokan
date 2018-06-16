#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define PAGE_SIZE     4096

/*
 * Channel : communication channel between hakase and friend
 * !!! Do not call Channel class directly. Use Channel::Accessor instead. !!!
 */
class Channel {
public:
  template <class T>
  T *GetRawPtr() {
    return reinterpret_cast<T *>(_address);
  }
  void Read(int offset, int8_t &data) { ReadSub(offset, data); }
  void Read(int offset, int16_t &data) { ReadSub(offset, data); }
  void Read(int offset, int32_t &data) { ReadSub(offset, data); }
  void Read(int offset, int64_t &data) { ReadSub(offset, data); }
  void Read(int offset, uint8_t &data) { ReadSub(offset, data); }
  void Read(int offset, uint16_t &data) { ReadSub(offset, data); }
  void Read(int offset, uint32_t &data) { ReadSub(offset, data); }
  void Read(int offset, uint64_t &data) { ReadSub(offset, data); }
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
  /*
   * Channel::Accesosr : Wrapper to access channel
   * This class provides safer access to channel.
   *
   * How to use:
   * Accessor ch_ac(ch, signal); // initialize Accessor
   * ch_ac.Write(0, data);       // write sending data
   * rval = ch_ac.Do();          // communicate and get return value
   * ch_ac.Read(0, data);        // read return data
   * 
   */
  class Accessor {
  public:
    Accessor(Channel &ch, int32_t type) : _type(type), _ch(ch) {
      for(size_t i = 0; i < sizeof(_buffer) / sizeof(_buffer[0]); i++) {
        _buffer[i] = 0;
      }
    }
    Accessor() = delete;
    void Write(int offset, int8_t data) { WriteSub(offset, data); }
    void Write(int offset, int16_t data) { WriteSub(offset, data); }
    void Write(int offset, int32_t data) { WriteSub(offset, data); }
    void Write(int offset, int64_t data) { WriteSub(offset, data); }
    void Write(int offset, uint8_t data) { WriteSub(offset, data); }
    void Write(int offset, uint16_t data) { WriteSub(offset, data); }
    void Write(int offset, uint32_t data) { WriteSub(offset, data); }
    void Write(int offset, uint64_t data) { WriteSub(offset, data); }
    void Read(int offset, int8_t &data) { ReadSub(offset, data); }
    void Read(int offset, int16_t &data) { ReadSub(offset, data); }
    void Read(int offset, int32_t &data) { ReadSub(offset, data); }
    void Read(int offset, int64_t &data) { ReadSub(offset, data); }
    void Read(int offset, uint8_t &data) { ReadSub(offset, data); }
    void Read(int offset, uint16_t &data) { ReadSub(offset, data); }
    void Read(int offset, uint32_t &data) { ReadSub(offset, data); }
    void Read(int offset, uint64_t &data) { ReadSub(offset, data); }
    int32_t Do() {
      _ch.Reserve();
      // TODO refactoring
      for(int i = 0; i < 4096 - 8; i++) {
        _ch.Write(i, _buffer[i]);
      }
      int32_t rval = _ch.SendSignal(_type);
      _signal_sended = true;
      for(int i = 0; i < 4096 - 8; i++) {
        uint8_t data;
        _ch.Read(i, data);
        _buffer[i] = data;
      }
      return rval;
    }
  private:
    template<class T>
    void WriteSub(int offset, T data) {
      assert(!_signal_sended);
      assert(offset < 4096 - 8);
      reinterpret_cast<T *>(_buffer)[offset / sizeof(T)] = data;
    }
    template<class T>
    void ReadSub(int offset, T &data) {
      assert(_signal_sended);
      assert(offset < 4096 - 8);
      data = reinterpret_cast<T *>(_buffer)[offset / sizeof(T)];
    }
    uint8_t _buffer[4096 - 8];
    const int32_t _type;
    Channel &_ch;
    bool _signal_sended = false;
  };
protected:
  Channel() {
  }
  char *_address;
 private:
  void Write(int offset, int8_t data) { WriteSub(offset, data); }
  void Write(int offset, int16_t data) { WriteSub(offset, data); }
  void Write(int offset, int32_t data) { WriteSub(offset, data); }
  void Write(int offset, int64_t data) { WriteSub(offset, data); }
  void Write(int offset, uint8_t data) { WriteSub(offset, data); }
  void Write(int offset, uint16_t data) { WriteSub(offset, data); }
  void Write(int offset, uint32_t data) { WriteSub(offset, data); }
  void Write(int offset, uint64_t data) { WriteSub(offset, data); }
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
