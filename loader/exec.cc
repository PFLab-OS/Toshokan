#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include "test.h"
#include "channel.h"

void deploy() {
  extern uint8_t _binary_raw_bin_start[];
  extern uint8_t _binary_raw_bin_end[];
  extern uint8_t _binary_raw_bin_size[];
  size_t binary_raw_bin_size = reinterpret_cast<size_t>(_binary_raw_bin_size);
  assert(_binary_raw_bin_start + binary_raw_bin_size == _binary_raw_bin_end);
  for(uint8_t *addr = _binary_raw_bin_start; addr < _binary_raw_bin_end; addr++) {
    
  }
}

int main(int argc, char **argv) {
  int configfd_h2f = open("/sys/module/friend_loader/call/h2f", O_RDWR);
  int configfd_f2h = open("/sys/module/friend_loader/call/f2h", O_RDWR);
  if(configfd_h2f < 0 || configfd_f2h < 0) {
    perror("Open call failed");
    return -1;
  }

  char *h2f_address = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, configfd_h2f, 0));
  char *f2h_address = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, configfd_f2h, 0));
  if (h2f_address == MAP_FAILED || f2h_address == MAP_FAILED) {
    perror("mmap operation failed");
    return -1;
  }
  F2H f2h(f2h_address);
  H2F h2f(h2f_address);

  deploy();
  
  if (h2f.SendSignal(3) != 0) {
    show_result(false);
    return -1;
  }

  /*  if (f2h.WaitNewSignal() != 1)*/ {
    show_result(false);
    return -1;
  }

  show_result(true);

  close(configfd_h2f);
  close(configfd_f2h);
  return 0;
}
