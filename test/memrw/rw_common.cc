// read by rw_large.cc & rw_small.cc
#include <time.h>
#include "../test.h"
#include "common/channel.h"
#include "common/memrw.h"

int test_main(F2H &f2h, H2F &h2f, int argc, const char **argv) {
  static const uint32_t kRead = 0;
  static const uint64_t address = 1024 * 1024;
  uint8_t buf[kDataSize];
  MemoryAccessor::Writer mw(h2f, address, buf, kDataSize);
  uint8_t *h2f_buf = h2f.GetRawPtr<uint8_t>() + 2048 / sizeof(uint8_t);

  FILE *fp;
  uint8_t debug_buf_before_write[kDataSize + MemoryAccessor::kTransferSize];
  uint8_t debug_buf[kDataSize + MemoryAccessor::kTransferSize];

  if ((fp = fopen("/sys/kernel/debug/friend_loader/memory", "rb")) == NULL) {
    return 1;
  }

  if (fseek(fp, address, SEEK_SET) != 0) {
    return 1;
  }

  {
    size_t buf_read = 0;
    while(buf_read < sizeof(debug_buf_before_write)) {
      buf_read += fread(debug_buf_before_write + buf_read, 1, sizeof(debug_buf_before_write) - buf_read, fp);
    }
  }

  fclose(fp); // to flush buffered data

  // generate random data
  srand((unsigned)time(NULL));
  for (size_t i = 0; i < kDataSize; i++) {
    buf[i] = rand() % 0xFF;
  }

  mw.Do().Unwrap();

  h2f.Reserve();

  h2f.Write(0, kRead);
  h2f.Write(8, address);
  h2f.Write(16, MemoryAccessor::kTransferSize);

  assert(h2f.SendSignal(4) == 0);

  if (memcmp(h2f_buf, buf, (kDataSize > MemoryAccessor::kTransferSize) ? MemoryAccessor::kTransferSize : kDataSize) != 0) {
    return 1;
  }

  // reopen
  if ((fp = fopen("/sys/kernel/debug/friend_loader/memory", "rb")) == NULL) {
    return 1;
  }

  if (fseek(fp, address, SEEK_SET) != 0) {
    return 1;
  }

  {
    size_t buf_read = 0;
    while(buf_read < sizeof(debug_buf)) {
      buf_read += fread(debug_buf + buf_read, 1, sizeof(debug_buf) - buf_read, fp);
    }
  }
  
  fclose(fp);

  if (memcmp(debug_buf, buf, kDataSize) != 0) {
    return 1;
  }
  
  if (memcmp(debug_buf + kDataSize, debug_buf_before_write + kDataSize, MemoryAccessor::kTransferSize) != 0) {
    return 1;
  }
  return 0;
}
