#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "common/channel.h"
#include "test.h"

#define DEPLOY_PHYS_ADDR_START 0x80000000UL
#define DEPLOY_PHYS_ADDR_END 0xC0000000UL
#define DEPLOY_PHYS_MEM_SIZE (DEPLOY_PHYS_ADDR_END - DEPLOY_PHYS_ADDR_START)

int main(int argc, const char **argv) {
  // Clear physical memory
  int physmemfd = open("/dev/mem", O_SYNC);
  char *mem = static_cast<char *>(mmap(nullptr, DEPLOY_PHYS_MEM_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE, physmemfd, DEPLOY_PHYS_ADDR_START));
  if (mem == MAP_FAILED) {
    perror("mmap operation failed");
    return 255;
  }

  memset(mem, 0, DEPLOY_PHYS_MEM_SIZE);

  close(physmemfd);

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
