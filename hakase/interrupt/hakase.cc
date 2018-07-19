#include "hakase.h"

bool InterruptController::ProcessInterrupt(int64_t &vnum) {
  vnum = _f2h.OldRead<int64_t>(0);
  _f2h.Return(0);

  if (_callback[vnum].callback != nullptr) {
    _callback[vnum].callback(_callback[vnum].arg);
    return true;
  }
  return false;
}

void InterruptController::Init() {

  for (int i = 0; i < kIntVectorNum; i++) {
    _callback[i].callback = nullptr;
  }
}

int InterruptController::SetIntCallback(int_callback callback, void *arg) {

  for (int vector = 32; vector < kIntVectorNum; vector++) {
    if (_callback[vector].callback == nullptr) {
      _callback[vector].callback = callback;
      _callback[vector].arg = arg;
      return vector;
    }
  }
  return ReservedIntVector::kError;
}

void InterruptController::SetExceptionCallback(int vector, int_callback callback, void *arg) {

  _callback[vector].callback = callback;
  _callback[vector].arg = arg;
}

