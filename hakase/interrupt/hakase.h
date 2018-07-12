#include "common/channel.h"

class InterruptController {
public:
  InterruptController(I2H &i2h) : _i2h(i2h) {
  }
  InterruptController() = delete;

  int WaitSignal(int16_t &type);

private:
  I2H &_i2h;
};

