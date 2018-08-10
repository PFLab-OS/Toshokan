// read by rw_large.cc & rw_small.cc
#include <time.h>
#include <stdlib.h>
#include "channel/hakase.h"
#include "memrw/hakase.h"
#include "tests/test.h"

int test_main(F2H &f2h, H2F &h2f, I2H &i2h, int argc, const char **argv) {
  static const uint64_t address = 1024 * 1024;
  uint8_t buf[kDataSize];

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
    while (buf_read < sizeof(debug_buf_before_write)) {
      buf_read += fread(debug_buf_before_write + buf_read, 1,
                        sizeof(debug_buf_before_write) - buf_read, fp);
    }
  }

  fclose(fp);  // to flush buffered data

  // generate random data
  srand((unsigned)time(NULL));
  for (size_t i = 0; i < kDataSize; i++) {
    buf[i] = rand() % 0xFF;
  }

  MemoryAccessor::Writer mw(h2f, 1, address, buf, kDataSize);
  mw.Do().Unwrap();

  uint8_t tmp_buf[kDataSize];
  MemoryAccessor::Reader mr(h2f, 1, address, tmp_buf, kDataSize);
  mr.Do().Unwrap();

  if (memcmp(tmp_buf, buf, kDataSize) != 0) {
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
    while (buf_read < sizeof(debug_buf)) {
      buf_read +=
          fread(debug_buf + buf_read, 1, sizeof(debug_buf) - buf_read, fp);
    }
  }

  fclose(fp);

  if (memcmp(debug_buf, buf, kDataSize) != 0) {
    return 1;
  }

  if (memcmp(debug_buf + kDataSize, debug_buf_before_write + kDataSize,
             MemoryAccessor::kTransferSize) != 0) {
    return 1;
  }
  return 0;
}
