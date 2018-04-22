#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "channel.h"

int main(int argc, char **argv) {
  int configfd_h2f = open("/sys/module/friend_loader/call/h2f", O_RDWR);
  int configfd_f2h = open("/sys/module/friend_loader/call/f2h", O_RDWR);
  if(configfd_h2f < 0 || configfd_f2h < 0) {
    perror("Open call failed");
    return -1;
  }

  char *h2f = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, configfd_h2f, 0));
  char *f2h = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, configfd_f2h, 0));
  if (h2f == MAP_FAILED || f2h == MAP_FAILED) {
    perror("mmap operation failed");
    return -1;
  }

  send_signal(h2f, 2);

  char string[100];
  int offset = 0;
  while(true) {
    wait_new_signal(f2h);
  
    if (get_type(f2h) != 2) {
      printf("test: failed\n");
      return -1;
    }
    
    uint8_t data;
    read(f2h, 0, data);
    set_type(f2h, 0);

    if (offset == 100) {
      printf("test: failed\n");
      return -1;
    }
    
    string[offset] = data;

    if (data == '\0') {
      break;
    }
    offset++;
  }

  if (strcmp("abc\n", string) == 0) {
    printf("test: OK\n");
  } else {
    printf("test: failed\n");
    return -1;
  }
  
  close(configfd_h2f);
  close(configfd_f2h);
  return 0;
}
