#include "hakase.h"

int InterruptController::WaitSignal() {
  _f2h.WaitNewSignal();
}
