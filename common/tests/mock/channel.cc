#include "CppUTestExt/MockSupport.h"
#include "channel/hakase.h"
#include "debug.h"

Channel::Channel() {
}

int Channel::SendSignal(int16_t type) {
  return mock("Channel").actualCall("SendSignal").withParameter("type", type).returnIntValue();
}

void Channel::Reserve(int16_t id) {
  mock("Channel").actualCall("Reserve").withParameter("id", id);
}

void Channel::Release() {
  mock("Channel").actualCall("Release");
}

void Channel::Write(int offset, uint8_t data) {
  Debug::channel_write_buffer[offset] = data;
}

uint8_t Channel::Read(int offset) {
  return Debug::channel_read_buffer[offset];
}
