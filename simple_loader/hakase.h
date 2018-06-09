#pragma once
#include <stdint.h>
#include <vector>
#include <memory>
#include "common/channel.h"
#include "common/result.h"

static const uint64_t kDeployAddressStart = 0x100000;

class SimpleLoader {
public:
  struct DeployData {
    DeployData(uint64_t offset, size_t size, uint8_t *buf) : _offset(offset), _size(size), _buf(buf) {
    }
    DeployData() = delete;
    ~DeployData() {
      delete _buf;
    }
    uint64_t _offset;
    size_t _size;
    uint8_t *_buf;
  };
  
  // How to use: See SimpleLoader::Deploy().
  class BinaryFile {
  public:
    // make BinaryFile & load from file
    static std::unique_ptr<BinaryFile> Load(const char *fname);
    // get binary data of loaded file
    std::unique_ptr<DeployData> GetData() {
      if (_it == _data.end()) {
        return std::unique_ptr<DeployData>();
      }
      std::unique_ptr<DeployData> dd(new DeployData(_offset, (*_it).first, (*_it).second));
      _offset += (*_it).first;
      _it++;
      return dd;
    }
  private:
    BinaryFile() {
    }
    std::vector<std::pair<size_t, uint8_t *>> _data;
    std::vector<std::pair<size_t, uint8_t *>>::iterator _it;
    uint64_t _offset = 0;
  };

  SimpleLoader(H2F &h2f, std::unique_ptr<BinaryFile> file) : _h2f(h2f), _file(std::move(file)) {
  }
  SimpleLoader() = delete;
  Result<bool> Deploy();
private:
  H2F &_h2f;
  std::unique_ptr<BinaryFile> _file;
};
