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
    src_ch = new Channel2(channel_buf, src_ch_id);
    dest_ch = new Channel2(channel_buf, dest_ch_id);
    another_ch = new Channel2(channel_buf, another_ch_id);
  }

  TEST_TEARDOWN() {
    delete another_ch;
    delete dest_ch;
    delete src_ch;
    delete [] channel_buf;
  }
  uint8_t *channel_buf;
  Channel2 *src_ch;
  Channel2 *dest_ch;
  Channel2 *another_ch;
  Channel2::Id src_ch_id = Channel2::Id(0);
  Channel2::Id dest_ch_id = Channel2::Id(1);
  Channel2::Id another_ch_id = Channel2::Id(2);
};

TEST(Channel2, FirstOneCanReserveChannel) {
  CHECK_FALSE(src_ch->Reserve().IsError());
}

TEST(Channel2, NoOneSent) {
  // src_ch->SendSignal(dest_ch_id, 1);
  CHECK(dest_ch->CheckIfNewSignalArrived().IsError());
}

TEST(Channel2, ReservedButNotSent) {
  src_ch->Reserve();
  // src_ch->SendSignal(dest_ch_id, 1);
  CHECK(dest_ch->CheckIfNewSignalArrived().IsError());
}

TEST(Channel2, NoOneReturned) {
  src_ch->Reserve();
  src_ch->SendSignal(dest_ch_id, 1);
  dest_ch->CheckIfNewSignalArrived();
  // dest_ch->Return(0);
  CHECK(src_ch->CheckIfReturned().IsError());
}

TEST(Channel2, SendReceiveSignalOnce) {
  int32_t signal = rand();
  
  src_ch->Reserve();
  src_ch->SendSignal(dest_ch_id, signal);
  CHECK_EQUAL(signal, dest_ch->CheckIfNewSignalArrived().Unwrap());
}

TEST(Channel2, SendReceiveSignalTwice) {
  for (int i = 0; i < 2; i++) {
    int32_t signal = rand();
  
    src_ch->Reserve();
    src_ch->SendSignal(dest_ch_id, signal);
    CHECK_EQUAL(signal, dest_ch->CheckIfNewSignalArrived().Unwrap());
    dest_ch->Return(0);
    src_ch->Release();
  }
}

TEST(Channel2, GetReturnValueOnce) {
  int32_t rval = rand();

  src_ch->Reserve();
  src_ch->SendSignal(dest_ch_id, 1);
  dest_ch->CheckIfNewSignalArrived();
  dest_ch->Return(rval);
  CHECK_EQUAL(rval, src_ch->CheckIfReturned().Unwrap());
}

TEST(Channel2, GetReturnValueTwice) {
  for(int i = 0; i < 2; i++) {
    int32_t rval = rand();

    src_ch->Reserve();
    src_ch->SendSignal(dest_ch_id, 1);
    dest_ch->CheckIfNewSignalArrived();
    dest_ch->Return(rval);
    CHECK_EQUAL(rval, src_ch->CheckIfReturned().Unwrap());
    src_ch->Release();
  }
}

TEST(Channel2, UnableToReserveReservedChannel) {
  src_ch->Reserve();
  CHECK(dest_ch->Reserve().IsError());
}

TEST(Channel2, MultipleReceiver) {
  src_ch->Reserve();
  src_ch->SendSignal(another_ch_id, 1);
  dest_ch->CheckIfNewSignalArrived().IsError();
  CHECK_FALSE(another_ch->CheckIfNewSignalArrived().IsError());
}

TEST(Channel2, SomeoneHasToReceive) {
  src_ch->Reserve();
  src_ch->SendSignal(another_ch_id, 1);
  CHECK(dest_ch->CheckIfNewSignalArrived().IsError());
}

TEST(Channel2, ReleaseBeforeReserving) {
  src_ch->Reserve();
  src_ch->SendSignal(dest_ch_id, 1);
  dest_ch->CheckIfNewSignalArrived();
  dest_ch->Return(0);
  src_ch->CheckIfReturned();

  CHECK(src_ch->Reserve().IsError());
}

TEST(Channel2, WriteRead) {
  int offset = rand() % Channel2::kDataSize;
  uint8_t data = rand() % 0xFF;
  src_ch->Reserve();
  src_ch->Write(offset, data);
  CHECK_EQUAL(data, src_ch->Read(offset));
}

TEST(Channel2, WriteReadOverChannelBuffer) {
  int offset = rand() % Channel2::kDataSize;
  uint8_t data = rand() % 0xFF;
  src_ch->Reserve();
  src_ch->Write(offset, data);
  src_ch->SendSignal(dest_ch_id, 1);
  dest_ch->CheckIfNewSignalArrived();
  CHECK_EQUAL(data, dest_ch->Read(offset));
}

//
// Do not write following patterns
//
TEST(Channel2, ReserveBeforeSendingSignal) {
  CHECK_THROWS(AssertException, src_ch->SendSignal(dest_ch_id, 1)); 
}

TEST(Channel2, DoNotReleaseIfNonReserved) {
  CHECK_THROWS(AssertException, src_ch->Release()); 
}

TEST(Channel2, DoNotSendSignal0) {
  src_ch->Reserve();

  CHECK_THROWS(AssertException, src_ch->SendSignal(dest_ch_id, 0));
}

