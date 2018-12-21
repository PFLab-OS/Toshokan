#include <assert.h>
#include "channel/hakase.h"
#include "common/_memory.h"
#include "memrw/hakase.h"
#include "tests/test.h"

int test_main(F2H &f2h, H2F &h2f, I2H &i2h, int argc, const char **argv) {
  static const uint64_t kAddress = 0;

  // signature: jmp TrampolineBinEntry; xchg %ax, &ax (see hakase/FriendLoader/trampoline_loader.c)
  const uint8_t signature[] = {
      0xeb, static_cast<int>(MemoryMap::kTrampolineBinEntry) - 2,
      0x66, 0x90};

  uint8_t buf[sizeof(signature) / sizeof(*signature)];
  MemoryAccessor::Reader mr(h2f, 1, kAddress, buf,
                            sizeof(signature) / sizeof(*signature));
  mr.Do().Unwrap();
  if (memcmp(buf, signature, sizeof(signature) / sizeof(*signature)) != 0) {
    return 1;
  }

  FILE *fp;
  char debug_buf[sizeof(signature) / sizeof(*signature)];
  size_t debug_buf_read = 0;

  if ((fp = fopen("/sys/kernel/debug/friend_loader/memory", "rb")) == NULL) {
    return 1;
  }

  while (debug_buf_read < sizeof(signature) / sizeof(*signature)) {
    debug_buf_read +=
        fread(debug_buf + debug_buf_read, 1,
              sizeof(signature) / sizeof(*signature) - debug_buf_read, fp);
  }

  if (memcmp(debug_buf, signature, sizeof(signature) / sizeof(*signature)) !=
      0) {
    return 1;
  }

  return 0;
}
