#include "../channel_accessor2.h"
#include <functional>
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

TEST_GROUP(CallerChannelAccessor) {
  TEST_SETUP() {}

  TEST_TEARDOWN() { mock().clear(); }

  // to hook CallerChannelAccessor::Do()
  template <int kBufSize>
  class DebugCallerChannelAccessor : public CallerChannelAccessor<kBufSize> {
   public:
    DebugCallerChannelAccessor(Channel2 &ch, Channel2::Id dest,
                               Channel2::Signal signal)
        : CallerChannelAccessor<kBufSize>(ch, dest, signal) {}
    void SetDo(std::function<void()> f) { _f = f; }
    virtual void Do() { _f(); }

   private:
    std::function<void()> _f;
  };

  struct Env {
    void Init() {
      _channel_buf = new uint8_t[Channel2::kBufAddress];
      Channel2::InitBuffer(_channel_buf);
      _caller_ch = new Channel2(_channel_buf, _caller_id);
      _callee_ch = new Channel2(_channel_buf, _callee_id);
    }
    ~Env() {
      delete _callee_ch;
      delete _caller_ch;
      delete[] _channel_buf;
    }
    uint8_t *_channel_buf = nullptr;
    Channel2::Id _caller_id = Channel2::Id(0);
    Channel2::Id _callee_id = Channel2::Id(1);
    Channel2 *_caller_ch = nullptr;
    Channel2 *_callee_ch = nullptr;
  };

  Channel2::Signal GenerateRandomSignal() {
    int32_t signal_val = rand();
    while (signal_val == 0) {
      signal_val = rand();
    }
    return Channel2::Signal(signal_val);
  }

  Channel2::Signal GetDummySignal() { return Channel2::Signal(1); }
};

TEST(CallerChannelAccessor, SetOffset) {
  CallerChannelAccessor<12>::Offset<int> offset(8);
}

TEST(CallerChannelAccessor, DoNotSetUnalignedOffset) {
  CHECK_THROWS(AssertException,
               ([]() { CallerChannelAccessor<10>::Offset<int> offset(3); })());
}

TEST(CallerChannelAccessor, DoNotSetOutRangeOffset) {
  CHECK_THROWS(AssertException,
               ([]() { CallerChannelAccessor<10>::Offset<int> offset(8); })());
}

TEST(CallerChannelAccessor, IsDoCalled) {
  Env env;
  env.Init();
  DebugCallerChannelAccessor<0> caller_ca(*env._caller_ch, env._callee_id,
                                          GetDummySignal());
  caller_ca.SetDo([&env]() {
    mock().actualCall("Do");

    env._callee_ch->Return(0);
  });
  mock().expectOneCall("Do");
  caller_ca.Call();  // CallerChannelAccessor::Do() will be called
  mock().checkExpectations();
}

TEST(CallerChannelAccessor, BlockUntilReturn) {
  int count = (rand() % 3) + 2;
  Env env;
  env.Init();
  DebugCallerChannelAccessor<0> caller_ca(*env._caller_ch, env._callee_id,
                                          GetDummySignal());
  caller_ca.SetDo([&env, &count]() {
    // do not return until count == 0
    if (count == 0) {
      env._callee_ch->Return(0);
    } else {
      count--;
    }
  });
  caller_ca.Call();
  CHECK_EQUAL(0, count);
}

TEST(CallerChannelAccessor, SendSignal) {
  Channel2::Signal signal = GenerateRandomSignal();
  Channel2::Signal arrived_signal =
      GenerateRandomSignal();  // dummy initialization

  Env env;
  env.Init();
  DebugCallerChannelAccessor<0> caller_ca(*env._caller_ch, env._callee_id,
                                          signal);
  caller_ca.SetDo([&env, &arrived_signal]() {
    // callee channel will get signal.
    arrived_signal = env._callee_ch->GetArrivedSignal();
    env._callee_ch->Return(0);
  });
  caller_ca.Call();
  CHECK(signal == arrived_signal);
}

TEST(CallerChannelAccessor, GetReturnValue) {
  int rval = rand();

  Env env;
  env.Init();
  DebugCallerChannelAccessor<0> caller_ca(*env._caller_ch, env._callee_id,
                                          GetDummySignal());
  caller_ca.SetDo([&env, rval]() {
    // callee channel returns value.
    env._callee_ch->Return(rval);
  });
  CHECK_EQUAL(rval, caller_ca.Call());
}

TEST(CallerChannelAccessor, Write) {
  static const int kBufSize = Channel2::kDataSize / sizeof(int);
  int send_buf[kBufSize];
  int receive_buf[kBufSize];
  for (int i = 0; i < kBufSize; i++) {
    send_buf[i] = rand();
  }

  Env env;
  env.Init();
  DebugCallerChannelAccessor<Channel2::kDataSize> caller_ca(
      *env._caller_ch, env._callee_id, GetDummySignal());

  for (int i = 0; i < kBufSize; i++) {
    // write to the buffer.
    caller_ca.Write(CallerChannelAccessor<Channel2::kDataSize>::Offset<int>(
                        i * sizeof(int)),
                    send_buf[i]);
  }

  caller_ca.SetDo([&env, &receive_buf]() {
    for (int i = 0; i < Channel2::kDataSize; i++) {
      // callee channel will reads from the buffer.
      (reinterpret_cast<uint8_t *>(receive_buf))[i] = env._callee_ch->Read(i);
    }
    env._callee_ch->Return(0);
  });
  caller_ca.Call();
  MEMCMP_EQUAL(send_buf, receive_buf, Channel2::kDataSize);
}

TEST(CallerChannelAccessor, Read) {
  static const int kBufSize = Channel2::kDataSize / sizeof(int);
  int send_buf[kBufSize];
  int receive_buf[kBufSize];
  for (int i = 0; i < kBufSize; i++) {
    send_buf[i] = rand();
  }
  Env env;
  env.Init();
  DebugCallerChannelAccessor<Channel2::kDataSize> caller_ca(
      *env._caller_ch, env._callee_id, GetDummySignal());
  caller_ca.SetDo([&env, send_buf]() {
    for (int i = 0; i < Channel2::kDataSize; i++) {
      // callee channel writes to the buffer.
      env._callee_ch->Write(i,
                            (reinterpret_cast<const uint8_t *>(send_buf))[i]);
    }
    env._callee_ch->Return(0);
  });
  caller_ca.Call();

  for (int i = 0; i < kBufSize; i++) {
    // read from the buffer.
    receive_buf[i] =
        caller_ca.Read(CallerChannelAccessor<Channel2::kDataSize>::Offset<int>(
            i * sizeof(int)));
  }
  MEMCMP_EQUAL(receive_buf, send_buf, Channel2::kDataSize);
}

TEST(CallerChannelAccessor, DoNotReadBeforeCall) {
  static const int kBufSize = Channel2::kDataSize / sizeof(int);
  int buf[kBufSize];

  Env env;
  env.Init();
  DebugCallerChannelAccessor<Channel2::kDataSize> caller_ca(
      *env._caller_ch, env._callee_id, GetDummySignal());

  CHECK_THROWS(AssertException,
               caller_ca.Read(
                   CallerChannelAccessor<Channel2::kDataSize>::Offset<int>(0)));
  // caller_ca.Call();
}

TEST(CallerChannelAccessor, DoNotWriteAfterCall) {
  static const int kBufSize = Channel2::kDataSize / sizeof(int);
  int buf[kBufSize];

  Env env;
  env.Init();
  DebugCallerChannelAccessor<Channel2::kDataSize> caller_ca(
      *env._caller_ch, env._callee_id, GetDummySignal());

  caller_ca.SetDo([&env]() { env._callee_ch->Return(0); });
  caller_ca.Call();
  CHECK_THROWS(
      AssertException,
      caller_ca.Write(
          CallerChannelAccessor<Channel2::kDataSize>::Offset<int>(0), 0));
}

TEST_GROUP(CalleeChannelAccessor) {
  TEST_SETUP() {}

  TEST_TEARDOWN() {}

  struct Env {
    void Init() {
      _channel_buf = new uint8_t[Channel2::kBufAddress];
      Channel2::InitBuffer(_channel_buf);
      _caller_ch = new Channel2(_channel_buf, _caller_id);
      _callee_ch = new Channel2(_channel_buf, _callee_id);
    }
    ~Env() {
      delete _callee_ch;
      delete _caller_ch;
      delete[] _channel_buf;
    }
    uint8_t *_channel_buf = nullptr;
    Channel2::Id _caller_id = Channel2::Id(0);
    Channel2::Id _callee_id = Channel2::Id(1);
    Channel2 *_caller_ch = nullptr;
    Channel2 *_callee_ch = nullptr;
  };

  Channel2::Signal GenerateRandomSignal() {
    int32_t signal_val = rand();
    while (signal_val == 0) {
      signal_val = rand();
    }
    return Channel2::Signal(signal_val);
  }

  Channel2::Signal GetDummySignal() { return Channel2::Signal(1); }
};

TEST(CalleeChannelAccessor, SetOffset) {
  CalleeChannelAccessor::Offset<int> offset(8);
}

TEST(CalleeChannelAccessor, DoNotSetUnalignedOffset) {
  CHECK_THROWS(AssertException,
               ([]() { CalleeChannelAccessor::Offset<int> offset(3); })());
}

TEST(CalleeChannelAccessor, DoNotSetOutRangeOffset) {
  CHECK_THROWS(AssertException, ([]() {
                 CalleeChannelAccessor::Offset<int> offset(
                     (((Channel2::kDataSize + 3) / 4) * 4) - 2);
               })());
}

TEST(CalleeChannelAccessor, GetSignal) {
  Channel2::Signal sent_signal = GenerateRandomSignal();

  Env env;
  env.Init();
  env._caller_ch->Reserve();
  env._caller_ch->SendSignal(env._callee_id, sent_signal);

  CalleeChannelAccessor callee_ca(*env._callee_ch);
  callee_ca.ReceiveSignal();
  CHECK(sent_signal == callee_ca.GetSignal());
}

TEST(CalleeChannelAccessor, ReturnValue) {
  int32_t rval = rand();

  Env env;
  env.Init();
  env._caller_ch->Reserve();
  env._caller_ch->SendSignal(env._callee_id, GetDummySignal());

  CalleeChannelAccessor callee_ca(*env._callee_ch);
  callee_ca.ReceiveSignal();
  callee_ca.Return(rval);
  CHECK_EQUAL(rval, env._caller_ch->CheckIfReturned().Unwrap());
}

TEST(CalleeChannelAccessor, DoNotGetSignalBeforeReceiveSignal) {
  Env env;
  env.Init();
  env._caller_ch->Reserve();
  env._caller_ch->SendSignal(env._callee_id, GetDummySignal());

  CalleeChannelAccessor callee_ca(*env._callee_ch);
  // callee_ca.ReceiveSignal();
  CHECK_THROWS(AssertException, callee_ca.GetSignal());
}

TEST(CalleeChannelAccessor, DoNotReturnBeforeReceiveSignal) {
  Env env;
  env.Init();
  env._caller_ch->Reserve();
  env._caller_ch->SendSignal(env._callee_id, GetDummySignal());

  CalleeChannelAccessor callee_ca(*env._callee_ch);
  // callee_ca.ReceiveSignal();
  CHECK_THROWS(AssertException, callee_ca.Return(0));
}

TEST(CalleeChannelAccessor, DoNotGetSignalAfterReturn) {
  Env env;
  env.Init();
  env._caller_ch->Reserve();
  env._caller_ch->SendSignal(env._callee_id, GetDummySignal());

  CalleeChannelAccessor callee_ca(*env._callee_ch);
  callee_ca.ReceiveSignal();
  callee_ca.Return(0);

  CHECK_THROWS(AssertException, callee_ca.GetSignal());
}

TEST(CalleeChannelAccessor, DoNotReturnAfterReturn) {
  Env env;
  env.Init();
  env._caller_ch->Reserve();
  env._caller_ch->SendSignal(env._callee_id, GetDummySignal());

  CalleeChannelAccessor callee_ca(*env._callee_ch);
  callee_ca.ReceiveSignal();
  callee_ca.Return(0);

  CHECK_THROWS(AssertException, callee_ca.Return(0));
}

TEST(CalleeChannelAccessor, Read) {
  static const int kBufSize = Channel2::kDataSize / sizeof(int);
  int send_buf[kBufSize];
  int receive_buf[kBufSize];
  for (int i = 0; i < kBufSize; i++) {
    send_buf[i] = rand();
  }

  Env env;
  env.Init();
  env._caller_ch->Reserve();
  for (int i = 0; i < Channel2::kDataSize; i++) {
    // write to the buffer.
    env._caller_ch->Write(i, (reinterpret_cast<const uint8_t *>(send_buf))[i]);
  }
  env._caller_ch->SendSignal(env._callee_id, GetDummySignal());

  CalleeChannelAccessor callee_ca(*env._callee_ch);
  callee_ca.ReceiveSignal();
  for (int i = 0; i < kBufSize; i++) {
    // read from the buffer;
    receive_buf[i] =
        callee_ca.Read(CalleeChannelAccessor::Offset<int>(i * sizeof(int)));
  }
  callee_ca.Return(0);

  MEMCMP_EQUAL(send_buf, receive_buf, Channel2::kDataSize);
}

TEST(CalleeChannelAccessor, Write) {
  static const int kBufSize = Channel2::kDataSize / sizeof(int);
  int send_buf[kBufSize];
  int receive_buf[kBufSize];
  for (int i = 0; i < kBufSize; i++) {
    send_buf[i] = rand();
  }

  Env env;
  env.Init();
  env._caller_ch->Reserve();
  env._caller_ch->SendSignal(env._callee_id, GetDummySignal());

  CalleeChannelAccessor callee_ca(*env._callee_ch);
  callee_ca.ReceiveSignal();
  for (int i = 0; i < kBufSize; i++) {
    // write to the buffer.
    callee_ca.Write(CalleeChannelAccessor::Offset<int>(i * sizeof(int)),
                    send_buf[i]);
  }
  callee_ca.Return(0);

  for (int i = 0; i < Channel2::kDataSize; i++) {
    // read from the buffer;
    (reinterpret_cast<uint8_t *>(receive_buf))[i] = env._caller_ch->Read(i);
  }

  MEMCMP_EQUAL(send_buf, receive_buf, Channel2::kDataSize);
}

TEST(CalleeChannelAccessor, DoNotReadAfterReturn) {
  Env env;
  env.Init();
  env._caller_ch->Reserve();
  env._caller_ch->SendSignal(env._callee_id, GetDummySignal());

  CalleeChannelAccessor callee_ca(*env._callee_ch);
  callee_ca.ReceiveSignal();
  callee_ca.Return(0);

  CHECK_THROWS(AssertException,
               callee_ca.Read(CalleeChannelAccessor::Offset<int>(0)));
}

TEST(CalleeChannelAccessor, DoNotWriteAfterReturn) {
  Env env;
  env.Init();
  env._caller_ch->Reserve();
  env._caller_ch->SendSignal(env._callee_id, GetDummySignal());

  CalleeChannelAccessor callee_ca(*env._callee_ch);
  callee_ca.ReceiveSignal();
  callee_ca.Return(0);

  CHECK_THROWS(AssertException,
               callee_ca.Write(CalleeChannelAccessor::Offset<int>(0), 0));
}
