#include "common/channel.h"

class InterruptController {
public:
  InterruptController(F2H &f2h) : _f2h(f2h) {
  }
  InterruptController() = delete;

  int WaitSignal(int16_t &type);

private:
  F2H &_f2h;
};

