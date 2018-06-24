#include "hakase.h"

int InterruptController::WaitSignal() {
  return _f2h.WaitNewSignal();
}
