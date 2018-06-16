#include <assert.h>
#include "tests/test.h"
#include "common/channel.h"
#include "memrw/hakase.h"

int test_main(F2H &f2h, H2F &h2f, int argc, const char **argv) {
  static const uint32_t kRead = 0;
  static const uint64_t kAddress = 0;
  const uint8_t signature[] = {0xeb, 0x1e, 0x66, 0x90, 0x6b, 0x72, 0x70, 0x4a};

  Channel::Accessor ch_ac(h2f, 4);
  ch_ac.Write(0, kRead);
  ch_ac.Write(8, kAddress);
  ch_ac.Write(16, sizeof(signature) / sizeof(*signature));
  
  assert(ch_ac.Do() == 0);

  for(size_t i = 0; i < sizeof(signature)/sizeof(*signature); i++) {
    uint8_t data;
    ch_ac.Read(i + 2040, data);
    if (data != signature[i]) {
      return 1;
    }
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
