#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include "../test.h"
#include "channel.h"
#include "memrw.h"

static const uint64_t kDeployAddressStart = 0x100000;

int deploy(H2F &h2f, const char *fname) {
  FILE *fp;

  fp = fopen(fname, "rb");
  if (fp == NULL){
    return 1;
  }

  std::vector<std::pair<size_t, uint8_t *>> data;

  while(true) {
    uint8_t *buf = new uint8_t[2048];
    size_t sz = fread(buf, 1, 2048, fp);
    if (sz == 0) {
      delete buf;
      break;
    }
    assert(sz < 2048);
    std::pair<size_t, uint8_t *> p = std::make_pair(sz, buf);
    data.push_back(p);
  }

  fclose(fp);

  uint64_t deploy_address = kDeployAddressStart;
  for (auto it = data.begin(); it != data.end(); it++) {
    MemoryAccessor::Writer mw(h2f, deploy_address, MemoryAccessor::DataSize::Create((*it).first).Unwrap());
    mw.Copy((*it).second).Unwrap();
    mw.Do().Unwrap();
    delete (*it).second;
    deploy_address += (*it).first;
  }

  return 0;
}

int test_main(F2H &f2h, H2F &h2f, int argc, const char **argv) {
  if (argc < 2) {
    return 1;
  }
  
  if (deploy(h2f, argv[1]) != 0) {
    return 1;
  }

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
