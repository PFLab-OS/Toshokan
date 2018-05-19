#include <string.h>
#include <assert.h>
#include "test.h"
#include "channel.h"

int test_main(F2H &f2h, H2F &h2f) {
  static const uint32_t kRead = 0;
  static const uint64_t address = 0;
  uint8_t *h2f_buf = h2f.GetRawPtr<uint8_t>() + 2048 / sizeof(uint8_t);
  const uint8_t signature[] = {0xeb, 0x1e, 0x66, 0x90, 0x6b, 0x72, 0x70, 0x4a};

  h2f.Reserve();
  
  h2f.Write(0, kRead);
  h2f.Write(8, address);

  assert(h2f.SendSignal(4) == 0);

  if (memcmp(h2f_buf, signature, sizeof(signature) / sizeof(*signature)) != 0) {
    return 1;
  }

  FILE *fp;
  char debug_buf[sizeof(signature) / sizeof(*signature)];
  size_t debug_buf_read = 0;

  if ((fp = fopen("/sys/kernel/debug/friend_loader/memory", "rb")) == NULL) {
    return 1;
  }

  while(debug_buf_read < sizeof(signature) / sizeof(*signature)) {
    debug_buf_read += fread(debug_buf + debug_buf_read, 1, sizeof(signature) / sizeof(*signature) - debug_buf_read, fp);
  }

  if (memcmp(debug_buf, signature, sizeof(signature) / sizeof(*signature)) != 0) {
    return 1;
  }

  return 0;
}
