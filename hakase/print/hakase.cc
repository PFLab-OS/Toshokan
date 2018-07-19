#include "hakase.h"

void StringReceiver::Do() {
  while(true) {
    int16_t type;
    _f2h.WaitNewSignal(type);
    if (type != 2) {
      return;
    }
    
    uint8_t data = _f2h.Read(0);
    _f2h.Return(0);

    if (data == '\0') {
      break;
    }
    
    _str->push_back(static_cast<char>(data));
  }
}
