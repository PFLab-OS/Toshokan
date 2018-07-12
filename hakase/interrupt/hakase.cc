#include "hakase.h"

int InterruptController::WaitSignal(int16_t &type) {
  return _i2h.WaitNewSignal(type);
}
