#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "channel.h"
#include "test.h"

int main(int argc, const char **argv) {
  int configfd_h2f = open("/sys/module/friend_loader/call/h2f", O_RDWR);
  int configfd_f2h = open("/sys/module/friend_loader/call/f2h", O_RDWR);
  if(configfd_h2f < 0 || configfd_f2h < 0) {
    perror("Open call failed");
    return 255;
  }

  char *h2f_address = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, configfd_h2f, 0));
  char *f2h_address = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, configfd_f2h, 0));
  if (h2f_address == MAP_FAILED || f2h_address == MAP_FAILED) {
    perror("mmap operation failed");
    return 255;
  }
  F2H f2h(f2h_address);
  H2F h2f(h2f_address);

  int rval = test_main(f2h, h2f, argc, argv);
  
  close(configfd_h2f);
  close(configfd_f2h);
  return rval;
}
