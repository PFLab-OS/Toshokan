#include <assert.h>
#include "../test.h"
#include "channel.h"
#include "memrw.h"

static const uint64_t kDeployAddressStart = 0x100000;

void deploy(H2F &h2f) {
  extern uint8_t _binary_raw_bin_start[];
  extern uint8_t _binary_raw_bin_end[];
  extern uint8_t _binary_raw_bin_size[];
  size_t binary_raw_bin_size = reinterpret_cast<size_t>(_binary_raw_bin_size);
  assert(_binary_raw_bin_start + binary_raw_bin_size == _binary_raw_bin_end);

  uint64_t deploy_address = kDeployAddressStart;
  for(uint8_t *addr = _binary_raw_bin_start; addr < _binary_raw_bin_end; addr += 2048, deploy_address += 2048) {
    MemoryAccessor::Writer mw(h2f, deploy_address, MemoryAccessor::DataSize::Create(_binary_raw_bin_end - addr > 2048 ? 2048 : _binary_raw_bin_end - addr).Unwrap());
    mw.Copy(addr).Unwrap();
    mw.Do().Unwrap();
  }
}

int test_main(F2H &f2h, H2F &h2f) {
  deploy(h2f);

  h2f.Reserve();
  h2f.Write(0, kDeployAddressStart);
  if (h2f.SendSignal(3) != 0) {
    return 1;
  }
  if (f2h.WaitNewSignal() != 1) {
    return 1;
  }

  return 0;
}
