using uint8_t = unsigned char;
using int32_t = int;

void panic();

int32_t get_type(char *channel) {
  return *((int32_t *)channel);
}

void set_type(char *channel, int32_t type) {
  *((int32_t *)channel) = type;
}

void read(char *channel, int offset, uint8_t &data) {
  data = channel[offset + 4];
}

void write(char *channel, int offset, uint8_t data) {
  channel[offset + 4] = data;
}

void write_string(char *channel, const char *str) {
  while(true) {
    while(get_type(channel) != 0) {
      asm volatile("":::"memory");
    }

    write(channel, 0, *str);
  
    set_type(channel, 2);
    
    if (*str == '\0') {
      break;
    }
    
    str++;
  }
}

bool do_signal() {
  char *h2f = (char *)0x2000;
  char *f2h = (char *)0x3000;
  switch(get_type(h2f)) {
  case 0:
    // NOP
    return true;
  case 1:
    // callback test
    set_type(h2f, 0);
    set_type(f2h, 1);
    return false;
  case 2:
    // print request test
    set_type(h2f, 0);

    write_string(f2h, "abc\n");
    return false;
  }
}

extern "C" void trampoline_main() {
  while(do_signal()) {
    asm volatile("":::"memory");
  }
  panic();
}

void panic() {
  while(true) {
    asm volatile("cli;hlt;nop;");
  }
}
