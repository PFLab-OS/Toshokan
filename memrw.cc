#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "test.h"
#include "channel.h"
#include "memrw.h"

int test_DataSize() {
  return 0; 
}

int test_invalid(F2H &f2h, H2F &h2f) {
  static const uint32_t kInvalid = 2;
  h2f.Reserve();
  h2f.Write(0, kInvalid);
  assert(h2f.SendSignal(4) < 0);
  return 0;
}

int test_of_reading_signature(F2H &f2h, H2F &h2f) {
  static const uint32_t kRead = 0;
  static const uint64_t address = 0;
  uint8_t *h2f_buf = h2f.GetRawPtr<uint8_t>() + 2048 / sizeof(uint8_t);
  const uint8_t signature[] = {0xeb, 0x1e, 0x66, 0x90, 0x6b, 0x72, 0x70, 0x4a};

  h2f.Reserve();
  
  h2f.Write(0, kRead);
  h2f.Write(8, address);

  assert(h2f.SendSignal(4) == 0);

  if (memcmp(h2f_buf, signature, sizeof(signature) / sizeof(*signature)) != 0) {
    return -1;
  }

  FILE *fp;
  char debug_buf[sizeof(signature) / sizeof(*signature)];
  size_t debug_buf_read = 0;

  if ((fp = fopen("/sys/kernel/debug/friend_loader/memory", "rb")) == NULL) {
    return -1;
  }

  while(debug_buf_read < sizeof(signature) / sizeof(*signature)) {
    debug_buf_read += fread(debug_buf + debug_buf_read, 1, sizeof(signature) / sizeof(*signature) - debug_buf_read, fp);
  }

  if (memcmp(debug_buf, signature, sizeof(signature) / sizeof(*signature)) != 0) {
    return -1;
  }

  return 0;
}

int test_of_rw(F2H &f2h, H2F &h2f) {
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
      return -1;
  }

  FILE *fp;
  char debug_buf[2048];
  size_t debug_buf_read = 0;

  if ((fp = fopen("/sys/kernel/debug/friend_loader/memory", "rb")) == NULL) {
    return -1;
  }

  if (fseek(fp, address, SEEK_SET) != 0) {
    return -1;
  }

  while(debug_buf_read < 2048) {
    debug_buf_read += fread(debug_buf + debug_buf_read, 1, 2048 - debug_buf_read, fp);
  }

  if (memcmp(debug_buf, buf, 2048) != 0) {
      return -1;
  }

  return 0;
}
  
int main(int argc, char **argv) {
  int configfd_h2f = open("/sys/module/friend_loader/call/h2f", O_RDWR);
  int configfd_f2h = open("/sys/module/friend_loader/call/f2h", O_RDWR);
  if(configfd_h2f < 0 || configfd_f2h < 0) {
    perror("Open call failed");
    return -1;
  }

  char *h2f_address = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, configfd_h2f, 0));
  char *f2h_address = static_cast<char *>(mmap(nullptr, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, configfd_f2h, 0));
  if (h2f_address == MAP_FAILED || f2h_address == MAP_FAILED) {
    perror("mmap operation failed");
    return -1;
  }
  F2H f2h(f2h_address);
  H2F h2f(h2f_address);

  if (test_DataSize() != 0) {
    show_result(false);
    return -1;
  }

  if (test_invalid(f2h, h2f) != 0) {
    show_result(false);
    return -1;
  }

  if (test_of_reading_signature(f2h, h2f) != 0) {
    show_result(false);
    return -1;
  }

  if (test_of_rw(f2h, h2f) != 0) {
    show_result(false);
    return -1;
  }
  
  show_result(true);
  
  close(configfd_h2f);
  close(configfd_f2h);
  return 0;
}
