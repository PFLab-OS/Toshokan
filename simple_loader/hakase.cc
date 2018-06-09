#include "hakase.h"
#include "common/memrw.h"
#include <stdio.h>
#include <assert.h>

Result<bool> SimpleLoader::Deploy() {
  while(true) {
    auto dd = _file->GetData();
    if (!dd) {
      return Result<bool>(true);
    }

    MemoryAccessor::Writer mw(_h2f, kDeployAddressStart + dd->_offset, dd->_buf, dd->_size);
    mw.Do().Unwrap();
  }
}

std::unique_ptr<SimpleLoader::BinaryFile> SimpleLoader::BinaryFile::Load(const char *fname) {
  std::unique_ptr<SimpleLoader::BinaryFile> that(new SimpleLoader::BinaryFile);
  FILE *fp;

  fp = fopen(fname, "rb");
  if (fp == NULL){
    return std::unique_ptr<SimpleLoader::BinaryFile>();
  }

  while(true) {
    uint8_t *buf = new uint8_t[2048];
    size_t sz = fread(buf, 1, 2048, fp);
    if (sz == 0) {
      delete buf;
      break;
    }
    assert(sz < 2048);
    std::pair<size_t, uint8_t *> p = std::make_pair(sz, buf);
    that->_data.push_back(p);
  }
  
  fclose(fp);
  that->_it = that->_data.begin();
  return that;
}
