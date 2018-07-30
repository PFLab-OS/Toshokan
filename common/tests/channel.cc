#include <future>
#include <string.h>
#include <chrono>
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "../channel2.h"

TEST_GROUP(Id) {
  TEST_SETUP() {
  }
  TEST_TEARDOWN() {
  }
};

TEST(Id, CheckEquality) {
  Channel2::Id id1(1);
  Channel2::Id id2(1);
  CHECK(id1 == id2);
}

TEST(Id, CheckSubstitution) {
  Channel2::Id id1(1);
  Channel2::Id id2 = id1;
  CHECK(id1 == id2);
}

TEST(Id, CheckCasSuccess) {
  Channel2::Id id1(1);
  Channel2::Id id2 = id1;
  Channel2::Id id3(2);
  CHECK(Channel2::Id::CompareAndSwap(&id1, id2, id3));
  CHECK(id1 == id3);
}

TEST(Id, CheckCasFail) {
  Channel2::Id id1(1);
  Channel2::Id id2 = id1;
  Channel2::Id id3(2);
  CHECK_FALSE(Channel2::Id::CompareAndSwap(&id1, id3, id3));
  CHECK(id1 == id2);
}

TEST_GROUP(Channel2) {
  TEST_SETUP() {
    channel_buf = new uint8_t[Channel2::kBufAddress];
    memset(channel_buf, 0, Channel2::kBufAddress);
    Channel2::InitBuffer(channel_buf);
  }

  TEST_TEARDOWN() {
    delete [] channel_buf;
    mock().clear();
  }
  uint8_t *channel_buf;
};

TEST(Channel2, FirstOneCanReserveChannel) {
  Channel2 ch(channel_buf, Channel2::Id(1));
  
  CHECK_FALSE(ch.Reserve().IsError());
}

TEST(Channel2, NoOneSent) {
  Channel2 ch(channel_buf, Channel2::Id(1));
  
  CHECK(ch.CheckIfNewSignalArrived().IsError());
}

TEST(Channel2, ReservedButNotSent) {
  Channel2 ch1(channel_buf, Channel2::Id(1));
  Channel2 ch2(channel_buf, Channel2::Id(0));

  ch1.Reserve();
  CHECK(ch2.CheckIfNewSignalArrived().IsError());
}

TEST(Channel2, NoOneReturned) {
  Channel2 ch1(channel_buf, Channel2::Id(1));
  Channel2 ch2(channel_buf, Channel2::Id(0));

  ch1.Reserve();
  ch1.SendSignal(Channel2::Id(0), 1);
  ch2.CheckIfNewSignalArrived();
  CHECK(ch1.CheckIfReturned().IsError());
}

TEST(Channel2, SendReceiveSignal1) {
  Channel2 ch1(channel_buf, Channel2::Id(1));
  Channel2 ch2(channel_buf, Channel2::Id(0));
  int32_t signal = rand();
  
  ch1.Reserve();
  ch1.SendSignal(Channel2::Id(0), signal);
  CHECK_EQUAL(signal, ch2.CheckIfNewSignalArrived().Unwrap());
}

TEST(Channel2, SendReceiveSignal2) {
  Channel2 ch1(channel_buf, Channel2::Id(1));
  Channel2 ch2(channel_buf, Channel2::Id(0));
  for (int i = 0; i < 2; i++) {
    int32_t signal = rand();
  
    ch1.Reserve();
    ch1.SendSignal(Channel2::Id(0), signal);
    CHECK_EQUAL(signal, ch2.CheckIfNewSignalArrived().Unwrap());
    ch2.Return(0);
    ch1.Release();
  }
}

TEST(Channel2, GetReturnValue1) {
  Channel2 ch1(channel_buf, Channel2::Id(1));
  Channel2 ch2(channel_buf, Channel2::Id(0));
  int32_t rval = rand();

  ch1.Reserve();
  ch1.SendSignal(Channel2::Id(0), 1);
  ch2.CheckIfNewSignalArrived();
  ch2.Return(rval);
  CHECK_EQUAL(rval, ch1.CheckIfReturned().Unwrap());
}

TEST(Channel2, GetReturnValue2) {
  Channel2 ch1(channel_buf, Channel2::Id(1));
  Channel2 ch2(channel_buf, Channel2::Id(0));
  for(int i = 0; i < 2; i++) {
    int32_t rval = rand();

    ch1.Reserve();
    ch1.SendSignal(Channel2::Id(0), 1);
    ch2.CheckIfNewSignalArrived();
    ch2.Return(rval);
    CHECK_EQUAL(rval, ch1.CheckIfReturned().Unwrap());
    ch1.Release();
  }
}

TEST(Channel2, UnableToReserveReservedChannel) {
  Channel2 ch1(channel_buf, Channel2::Id(1));
  Channel2 ch2(channel_buf, Channel2::Id(0));
  
  ch1.Reserve();
  CHECK(ch2.Reserve().IsError());
}

TEST(Channel2, MultipleReceiver) {
  Channel2 ch1(channel_buf, Channel2::Id(0));
  Channel2 ch2(channel_buf, Channel2::Id(1));
  Channel2 ch3(channel_buf, Channel2::Id(2));

  ch1.Reserve();
  ch1.SendSignal(Channel2::Id(2), 1);
  ch2.CheckIfNewSignalArrived().IsError();
  CHECK_FALSE(ch3.CheckIfNewSignalArrived().IsError());
}

TEST(Channel2, SomeoneHasToReceive) {
  Channel2 ch1(channel_buf, Channel2::Id(0));
  Channel2 ch2(channel_buf, Channel2::Id(1));
  ch2.Reserve();
  ch2.SendSignal(Channel2::Id(2), 1);
  CHECK(ch1.CheckIfNewSignalArrived().IsError());
}

TEST(Channel2, ReleaseBeforeReserving) {
  Channel2 ch1(channel_buf, Channel2::Id(1));
  Channel2 ch2(channel_buf, Channel2::Id(0));
  ch1.Reserve();
  ch1.SendSignal(Channel2::Id(0), 1);
  ch2.CheckIfNewSignalArrived();
  ch2.Return(0);
  ch1.CheckIfReturned();

  CHECK(ch1.Reserve().IsError());
}

TEST(Channel2, WriteRead) {
  int offset = rand() % Channel2::kDataSize;
  uint8_t data = rand() % 0xFF;
  Channel2 ch(channel_buf, Channel2::Id(1));
  ch.Reserve();
  ch.Write(offset, data);
  CHECK_EQUAL(data, ch.Read(offset));
}

TEST(Channel2, WriteReadOverChannelBuffer) {
  int offset = rand() % Channel2::kDataSize;
  uint8_t data = rand() % 0xFF;
  Channel2 ch1(channel_buf, Channel2::Id(1));
  Channel2 ch2(channel_buf, Channel2::Id(0));
  ch1.Reserve();
  ch1.Write(offset, data);
  ch1.SendSignal(Channel2::Id(0), 1);
  ch2.CheckIfNewSignalArrived();
  CHECK_EQUAL(data, ch2.Read(offset));
}

//
// Do not write following patterns
//
TEST(Channel2, ReserveBeforeSendingSignal) {
  Channel2 ch(channel_buf, Channel2::Id(1));
  
  mock().expectOneCall("assert");
  ch.SendSignal(Channel2::Id(0), 1);
  mock().checkExpectations();
}

TEST(Channel2, DoNotReleaseIfNonReserved) {
  Channel2 ch(channel_buf, Channel2::Id(1));
  
  mock().expectOneCall("assert");
  ch.Release();
  mock().checkExpectations();
}

TEST(Channel2, DoNotSendSignal0) {
  Channel2 ch(channel_buf, Channel2::Id(1));
  ch.Reserve();
  
  mock().expectOneCall("assert");
  ch.SendSignal(Channel2::Id(0), 0);
  mock().checkExpectations();
}

