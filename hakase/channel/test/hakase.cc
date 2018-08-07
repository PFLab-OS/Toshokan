#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "tests/test.h"

int test_main(F2H &f2h, H2F &h2f, I2H &i2h, int argc, const char **argv) {
  int configfd_h2f = open("/sys/module/friend_loader/call/h2f", O_RDWR);
  int configfd_f2h = open("/sys/module/friend_loader/call/f2h", O_RDWR);
  int configfd_i2h = open("/sys/module/friend_loader/call/i2h", O_RDWR);
  if(configfd_h2f < 0 || configfd_f2h < 0 || configfd_i2h < 0) {
    perror("Open call failed");
    return 255;
  }

  char *h2f_address = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, configfd_h2f, 0));
  char *f2h_address = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, configfd_f2h, 0));
  char *i2h_address = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, configfd_i2h, 0));
  if (h2f_address == MAP_FAILED || f2h_address == MAP_FAILED || i2h_address == MAP_FAILED) {
    perror("mmap operation failed");
    return 255;
  }

  int rval = 0;
  for (int i = 0; i < 4096; i++) {
    if (h2f_address[i] != 0 || f2h_address[i] != 0 || i2h_address[i] != 0) {
      rval = 1;
    }
  }
  
  close(configfd_h2f);
  close(configfd_f2h);
  close(configfd_i2h);
  return rval;
}

