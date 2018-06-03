#include <time.h>
#include "../test.h"
#include "channel.h"
#include "memrw.h"

int test_main(F2H &f2h, H2F &h2f) {
  static const uint32_t kRead = 0;
  static const uint64_t address = 1024 * 1024;
  MemoryAccessor::Writer mw(h2f, address, MemoryAccessor::DataSize::Create(2048).Unwrap());
  uint8_t *h2f_buf = h2f.GetRawPtr<uint8_t>() + 2048 / sizeof(uint8_t);

  // generate random data
  uint8_t buf[2048];
  srand((unsigned)time(NULL));
  for (int i = 0; i < 2048; i++) {
    buf[i] = rand() % 0xFF;
  }

  mw.Copy(buf).Unwrap();
  mw.Do().Unwrap();

  h2f.Reserve();

  h2f.Write(0, kRead);
  h2f.Write(8, address);

  assert(h2f.SendSignal(4) == 0);

  if (memcmp(h2f_buf, buf, 2048) != 0) {
    return 1;
  }

  FILE *fp;
  char debug_buf[2048];
  size_t debug_buf_read = 0;

  if ((fp = fopen("/sys/kernel/debug/friend_loader/memory", "rb")) == NULL) {
    return 1;
  }

  if (fseek(fp, address, SEEK_SET) != 0) {
    return 1;
  }

  while(debug_buf_read < 2048) {
    debug_buf_read += fread(debug_buf + debug_buf_read, 1, 2048 - debug_buf_read, fp);
  }

  if (memcmp(debug_buf, buf, 2048) != 0) {
    return 1;
  }
  return 0;
}
