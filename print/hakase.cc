#include "hakase.h"

void StringReceiver::Do() {
  while(true) {
    if (_f2h.WaitNewSignal() != 2) {
      return;
    }
    
    uint8_t data;
    _f2h.Read(0, data);
    _f2h.Return(0);

    if (data == '\0') {
      break;
    }
    
    _str->push_back(static_cast<char>(data));
  }
}
