#include "hakase.h"

int InterruptController::WaitSignal(int16_t &type) {
  return _f2h.WaitNewSignal(type);
}
