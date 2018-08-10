#include <memory>
#include <string>
#include "channel/hakase.h"

class StringReceiver {
 public:
  StringReceiver(F2H &f2h) : _f2h(f2h), _str(new std::string) {}
  StringReceiver() = delete;
  void Do();
  std::unique_ptr<std::string> GetString() { return std::move(_str); }

 private:
  F2H &_f2h;
  std::unique_ptr<std::string> _str;
};
