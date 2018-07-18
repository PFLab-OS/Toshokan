#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include <stdlib.h>
#include <string.h>
#include "../align.h"
#include "mock/debug.h"
#include "../channel_accessor.h"

TEST_GROUP(ChannelAccessor) {
  TEST_SETUP() {
  }

  TEST_TEARDOWN() {
    mock().clear();
  }
};

TEST(ChannelAccessor, Do) {
  const int16_t kType = rand();
  const int kReturnValue = rand();
  mock("Channel").expectOneCall("SendSignal").withParameter("type", kType).andReturnValue(kReturnValue);
  mock("Channel").ignoreOtherCalls();

  Channel ch;
  ChannelAccessor<> ch_ac(ch, kType);
  CHECK_EQUAL(kReturnValue, ch_ac.Do(0));

  mock().checkExpectations();
}

TEST(ChannelAccessor, Id) {
  const int16_t kId = rand();
  mock("Channel").expectOneCall("Reserve").withParameter("id", kId);
  mock("Channel").ignoreOtherCalls();

  Channel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Do(kId);

  mock().checkExpectations();
}

TEST(ChannelAccessor, ReadWrite0) {
  Debug::InitChannelBuffer();
  mock("Channel").ignoreOtherCalls();
  
  int16_t expected[kBufMax / sizeof(int16_t)];
  memset(expected, 0, sizeof(expected));

  Channel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Do(0);

  MEMCMP_EQUAL(expected, Debug::channel_write_buffer, kBufMax);

  mock().checkExpectations();
}

TEST(ChannelAccessor, Write1) {
  Debug::InitChannelBuffer();
  mock("Channel").ignoreOtherCalls();
  
  const int offset = align<int>((rand() % kBufMax), sizeof(int16_t));
  int16_t data = rand() % kBufMax;
  int16_t expected[kBufMax / sizeof(int16_t)];
  static_assert(sizeof(expected) == kBufMax, "");
  memset(expected, 0, sizeof(expected));
  expected[offset / sizeof(int16_t)] = data;

  Channel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Write<int16_t>(offset, data);
  ch_ac.Do(0);

  MEMCMP_EQUAL(expected, Debug::channel_write_buffer, kBufMax);

  mock().checkExpectations();
}

TEST(ChannelAccessor, WriteN) {
  Debug::InitChannelBuffer();
  mock("Channel").ignoreOtherCalls();

  int16_t expected[kBufMax / sizeof(int16_t)];
  memset(expected, 0, sizeof(expected));

  Channel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  for (int i = 0; i < (rand() % kBufMax); i++) {
    const int offset = align<int>((rand() % kBufMax), sizeof(int16_t));
    int16_t data = rand() % kBufMax;
    expected[offset / sizeof(int16_t)] = data;
    ch_ac.Write<int16_t>(offset, data);
  }
  ch_ac.Do(0);

  MEMCMP_EQUAL(expected, Debug::channel_write_buffer, kBufMax);

  mock().checkExpectations();
}

TEST(ChannelAccessor, Read1) {
  Debug::InitChannelBuffer();
  mock("Channel").ignoreOtherCalls();
  
  const int offset = align<int>((rand() % kBufMax), sizeof(int16_t));

  Channel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Do(0);
  CHECK_EQUAL(reinterpret_cast<int16_t *>(Debug::channel_read_buffer)[offset / sizeof(int16_t)], ch_ac.Read<int16_t>(offset));

  mock().checkExpectations();
}

TEST(ChannelAccessor, ReadN) {
  Debug::InitChannelBuffer();
  mock("Channel").ignoreOtherCalls();
  
  Channel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Do(0);
  for (int i = 0; i < (rand() % kBufMax); i++) {
    const int offset = align<int>((rand() % kBufMax), sizeof(int16_t));
    CHECK_EQUAL(reinterpret_cast<int16_t *>(Debug::channel_read_buffer)[offset / sizeof(int16_t)], ch_ac.Read<int16_t>(offset));
  }

  mock().checkExpectations();
}

//
// Do not write following patterns
//

// Offset must be aligned.
TEST(ChannelAccessor, WriteAlign) {
  mock().expectOneCall("assert");
  mock("Channel").ignoreOtherCalls();
  
  const int offset = align<int>((rand() % kBufMax), sizeof(int16_t)) + 1;

  Channel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Write<int16_t>(offset, 0);
  ch_ac.Do(0);

  mock().checkExpectations();
}

// Offset must be aligned.
TEST(ChannelAccessor, ReadAlign) {
  mock().expectOneCall("assert");
  mock("Channel").ignoreOtherCalls();
  
  const int offset = align<int>((rand() % kBufMax), sizeof(int16_t)) + 1;

  Channel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Do(0);
  ch_ac.Read<int16_t>(offset);

  mock().checkExpectations();
}

// Write() must be called before Do()
TEST(ChannelAccessor, WriteAfterDo) {
  mock().expectOneCall("assert");
  mock("Channel").ignoreOtherCalls();
  
  Channel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Do(0);
  ch_ac.Write<int16_t>(0, 0);

  mock().checkExpectations();
}

// Read() must be called after Do()
TEST(ChannelAccessor, ReadBeforeDo) {
  mock().expectOneCall("assert");
  mock("Channel").ignoreOtherCalls();
  
  Channel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Read<int16_t>(0);
  ch_ac.Do(0);

  mock().checkExpectations();
}

// Offset must be smaller than kBufSize
TEST(ChannelAccessor, WriteOffset) {
  mock().expectOneCall("assert");
  mock("Channel").ignoreOtherCalls();
  
  const int offset = kBufMax;

  Channel ch;
  ChannelAccessor<kBufMax - 1> ch_ac(ch, 0);
  ch_ac.Write<int16_t>(offset, 0);
  ch_ac.Do(0);

  mock().checkExpectations();
}

// Offset must be smaller than kBufSize
TEST(ChannelAccessor, ReadOffset) {
  mock().expectOneCall("assert");
  mock("Channel").ignoreOtherCalls();
  
  const int offset = kBufMax;

  Channel ch;
  ChannelAccessor<kBufMax - 1> ch_ac(ch, 0);
  ch_ac.Do(0);
  ch_ac.Read<int16_t>(offset);

  mock().checkExpectations();
}
