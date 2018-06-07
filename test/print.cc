#include <string.h>
#include "test.h"
#include "common/channel.h"

int test_main(F2H &f2h, H2F &h2f, int argc, const char **argv) {
  h2f.Reserve();
  h2f.SendSignal(2);

  char string[100];
  int offset = 0;
  while(true) {
    if (f2h.WaitNewSignal() != 2) {
      return 1;
    }
    
    uint8_t data;
    f2h.Read(0, data);
    f2h.Return(0);

    if (offset == 100) {
      return 1;
    }
    
    string[offset] = data;

    if (data == '\0') {
      break;
    }
    offset++;
  }

  if (strcmp("abc\n", string) != 0) {
    return 1;
  }
  
  return 0;
}
