#include <future>
#include <string.h>
#include <chrono>
#include "CppUTest/TestHarness.h"
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
    caller_ch = new Channel2(channel_buf, caller_ch_id);
    callee_ch = new Channel2(channel_buf, callee_ch_id);
    another_ch = new Channel2(channel_buf, another_ch_id);
  }

  TEST_TEARDOWN() {
    delete another_ch;
    delete callee_ch;
    delete caller_ch;
    delete [] channel_buf;
  }
  uint8_t *channel_buf;
  Channel2 *caller_ch;
  Channel2 *callee_ch;
  Channel2 *another_ch;
  Channel2::Id caller_ch_id = Channel2::Id(0);
  Channel2::Id callee_ch_id = Channel2::Id(1);
  Channel2::Id another_ch_id = Channel2::Id(2);
};

TEST(Channel2, FirstOneCanReserveChannel) {
  CHECK_FALSE(caller_ch->Reserve().IsError());
}

TEST(Channel2, NoOneSent) {
  // caller_ch->SendSignal(callee_ch_id, 1);
  CHECK(callee_ch->CheckIfNewSignalArrived().IsError());
}

TEST(Channel2, ReservedButNotSent) {
  caller_ch->Reserve();
  // caller_ch->SendSignal(callee_ch_id, 1);
  CHECK(callee_ch->CheckIfNewSignalArrived().IsError());
}

TEST(Channel2, NoOneReturned) {
  caller_ch->Reserve();
  caller_ch->SendSignal(callee_ch_id, 1);
  callee_ch->CheckIfNewSignalArrived();
  // callee_ch->Return(0);
  CHECK(caller_ch->CheckIfReturned().IsError());
}

TEST(Channel2, SendReceiveSignalOnce) {
  int32_t signal = rand();
  
  caller_ch->Reserve();
  caller_ch->SendSignal(callee_ch_id, signal);
  CHECK_EQUAL(signal, callee_ch->CheckIfNewSignalArrived().Unwrap());
}

TEST(Channel2, SendReceiveSignalTwice) {
  for (int i = 0; i < 2; i++) {
    int32_t signal = rand();
  
    caller_ch->Reserve();
    caller_ch->SendSignal(callee_ch_id, signal);
    CHECK_EQUAL(signal, callee_ch->CheckIfNewSignalArrived().Unwrap());
    callee_ch->Return(0);
    caller_ch->Release();
  }
}

TEST(Channel2, GetReturnValueOnce) {
  int32_t rval = rand();

  caller_ch->Reserve();
  caller_ch->SendSignal(callee_ch_id, 1);
  callee_ch->CheckIfNewSignalArrived();
  callee_ch->Return(rval);
  CHECK_EQUAL(rval, caller_ch->CheckIfReturned().Unwrap());
}

TEST(Channel2, GetReturnValueTwice) {
  for(int i = 0; i < 2; i++) {
    int32_t rval = rand();

    caller_ch->Reserve();
    caller_ch->SendSignal(callee_ch_id, 1);
    callee_ch->CheckIfNewSignalArrived();
    callee_ch->Return(rval);
    CHECK_EQUAL(rval, caller_ch->CheckIfReturned().Unwrap());
    caller_ch->Release();
  }
}

TEST(Channel2, UnableToReserveReservedChannel) {
  caller_ch->Reserve();
  CHECK(callee_ch->Reserve().IsError());
}

TEST(Channel2, MultipleReceiver) {
  caller_ch->Reserve();
  caller_ch->SendSignal(another_ch_id, 1);
  callee_ch->CheckIfNewSignalArrived().IsError();
  CHECK_FALSE(another_ch->CheckIfNewSignalArrived().IsError());
}

TEST(Channel2, SomeoneHasToReceive) {
  caller_ch->Reserve();
  caller_ch->SendSignal(another_ch_id, 1);
  CHECK(callee_ch->CheckIfNewSignalArrived().IsError());
}

TEST(Channel2, ReleaseBeforeReserving) {
  caller_ch->Reserve();
  caller_ch->SendSignal(callee_ch_id, 1);
  callee_ch->CheckIfNewSignalArrived();
  callee_ch->Return(0);
  caller_ch->CheckIfReturned();

  CHECK(caller_ch->Reserve().IsError());
}

TEST(Channel2, WriteRead) {
  int offset = rand() % Channel2::kDataSize;
  uint8_t data = rand() % 0xFF;
  caller_ch->Reserve();
  caller_ch->Write(offset, data);
  CHECK_EQUAL(data, caller_ch->Read(offset));
}

TEST(Channel2, WriteReadOverChannelBuffer) {
  int offset = rand() % Channel2::kDataSize;
  uint8_t data = rand() % 0xFF;
  caller_ch->Reserve();
  caller_ch->Write(offset, data);
  caller_ch->SendSignal(callee_ch_id, 1);
  callee_ch->CheckIfNewSignalArrived();
  CHECK_EQUAL(data, callee_ch->Read(offset));
}

//
// Do not write following patterns
//
TEST(Channel2, ReserveBeforeSendingSignal) {
  CHECK_THROWS(AssertException, caller_ch->SendSignal(callee_ch_id, 1)); 
}

TEST(Channel2, DoNotReleaseIfNonReserved) {
  CHECK_THROWS(AssertException, caller_ch->Release()); 
}

TEST(Channel2, DoNotSendSignal0) {
  caller_ch->Reserve();

  CHECK_THROWS(AssertException, caller_ch->SendSignal(callee_ch_id, 0));
}

