using uint8_t = unsigned char;
using int32_t = int;

void panic();

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

// callback test
void callback(H2F &h2f, F2H &f2h) {
  h2f.SetType(0);
  f2h.SetType(1);
}

// print request test
void print(H2F &h2f, F2H &f2h) {
  h2f.SetType(0);
  f2h.WriteString("abc\n");
}

// execute binary
void exec_bin(H2F &h2f, F2H &f2h) { h2f.SetType(-1); }

void do_signal(H2F &h2f) {
  F2H f2h;
  switch (h2f.GetType()) {
  case 0:
    panic();
  case 1:
    callback(h2f, f2h);
    break;
  case 2:
    print(h2f, f2h);
    break;
  case 3:
    exec_bin(h2f, f2h);
    break;
  }
}

extern "C" void trampoline_main() {
  H2F h2f;
  h2f.WaitNewSignal();
  do_signal(h2f);
  panic();
}

void panic() {
  while (true) {
    asm volatile("cli;hlt;nop;");
  }
}
