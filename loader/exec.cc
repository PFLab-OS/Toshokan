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

int test_main(F2H &f2h, H2F &h2f) {
  h2f.Reserve();

  deploy();
  
  if (h2f.SendSignal(3) != 0) {
    return 1;
  }

  /*  if (f2h.WaitNewSignal() != 1)*/ {
    return 1;
  }

  return 0;
}
