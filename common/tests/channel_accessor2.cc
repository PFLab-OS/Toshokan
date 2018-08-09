#include "../channel_accessor2.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include <functional>
#include <memory>

TEST_GROUP(CallerChannelAccessor) {
  TEST_SETUP() {
  }

  TEST_TEARDOWN() {
    mock().clear();
  }

  // to hook CallerChannelAccessor::Do()
  template<int kBufSize>
    class DebugCallerChannelAccessor : public CallerChannelAccessor<kBufSize> {
  public:
    DebugCallerChannelAccessor(std::function<void()> f, Channel2 &ch, Channel2::Id dest, Channel2::Signal signal) : CallerChannelAccessor<kBufSize>(ch, dest, signal), _f(f) {
    }
    virtual void Do() {
      _f();
    }
  private:
    std::function<void()> _f;
  };

  struct CallerChannelAccessorEnv {
    void Initialize() {
      _channel_buf = new uint8_t[Channel2::kBufAddress];
      Channel2::InitBuffer(_channel_buf);
      _caller_ch = new Channel2(_channel_buf, _caller_id);
      _callee_ch = new Channel2(_channel_buf, _callee_id);
    }
    ~CallerChannelAccessorEnv() {
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
  
  Channel2::Signal GetDummySignal() {
    return Channel2::Signal(1);
  }
};

TEST(CallerChannelAccessor, SetOffset) {
  CallerChannelAccessor<12>::Offset<int> offset(8);
}

TEST(CallerChannelAccessor, DoNotSetUnalignedOffset) {
  CHECK_THROWS(AssertException, ([]() {
        CallerChannelAccessor<10>::Offset<int> offset(3);
      })());
}

TEST(CallerChannelAccessor, DoNotSetOutRangeOffset) {
  CHECK_THROWS(AssertException, ([]() {
        CallerChannelAccessor<10>::Offset<int> offset(8);
      })());
}

TEST(CallerChannelAccessor, IsDoCalled) {
  CallerChannelAccessorEnv env;
  env.Initialize();
  DebugCallerChannelAccessor<0> caller_ca([&env]() {
      mock().actualCall("Do");
      
      env._callee_ch->Return(0);
    }, *env._caller_ch, env._callee_id, GetDummySignal());
  
  mock().expectOneCall("Do");
  caller_ca.Call(); // CallerChannelAccessor::Do() will be called
  mock().checkExpectations();
}

TEST(CallerChannelAccessor, BlockUntilReturn) {
  int count = (rand() % 3) + 2;
  CallerChannelAccessorEnv env;
  env.Initialize();
  DebugCallerChannelAccessor<0> caller_ca([&env, &count]() {
      // do not return until count == 0
      if (count == 0) {
        env._callee_ch->Return(0);
      } else {
        count--;
      }
    }, *env._caller_ch, env._callee_id, GetDummySignal());
  caller_ca.Call();
  CHECK_EQUAL(0, count);
}

TEST(CallerChannelAccessor, SendSignal) {
  Channel2::Signal signal = GenerateRandomSignal();
  std::unique_ptr<Channel2::Signal> sent_signal;
  
  CallerChannelAccessorEnv env;
  env.Initialize();
  DebugCallerChannelAccessor<0> caller_ca([&env, &sent_signal]() {
      // callee channel will get signal.
      sent_signal = std::unique_ptr<Channel2::Signal>(new Channel2::Signal(env._callee_ch->CheckIfNewSignalArrived().Unwrap()));
      env._callee_ch->Return(0); 
    }, *env._caller_ch, env._callee_id, signal);
  caller_ca.Call();
  CHECK(signal == *sent_signal);
}

TEST(CallerChannelAccessor, GetReturnValue) {
  int rval = rand();
  
  CallerChannelAccessorEnv env;
  env.Initialize();
  DebugCallerChannelAccessor<0> caller_ca([&env, rval]() {
      // callee channel returns value.
      env._callee_ch->Return(rval);
    }, *env._caller_ch, env._callee_id, GetDummySignal());
  CHECK_EQUAL(rval, caller_ca.Call());
}

TEST(CallerChannelAccessor, Write) {
  static const int kBufSize = Channel2::kDataSize / sizeof(int);
  int send_buf[kBufSize];
  int receive_buf[kBufSize];
  for(int i = 0; i < kBufSize; i++) {
    send_buf[i] = rand();
  }
  
  CallerChannelAccessorEnv env;
  env.Initialize();
  DebugCallerChannelAccessor<Channel2::kDataSize> caller_ca([&env, &receive_buf]() {
      for(int i = 0; i < Channel2::kDataSize; i++) {
        // callee channel will reads from the buffer.
        (reinterpret_cast<uint8_t *>(receive_buf))[i] = env._callee_ch->Read(i);
      }
      env._callee_ch->Return(0);
    }, *env._caller_ch, env._callee_id, GetDummySignal());
  
  for(int i = 0; i < kBufSize; i++) {
    // write to the buffer.
    caller_ca.Write(CallerChannelAccessor<Channel2::kDataSize>::Offset<int>(i * sizeof(int)), send_buf[i]);
  }
  caller_ca.Call();
  MEMCMP_EQUAL(send_buf, receive_buf, Channel2::kDataSize);
}

TEST(CallerChannelAccessor, Read) {
  static const int kBufSize = Channel2::kDataSize / sizeof(int);
  int send_buf[kBufSize];
  int receive_buf[kBufSize];
  for(int i = 0; i < kBufSize; i++) {
    send_buf[i] = rand();
  }
  CallerChannelAccessorEnv env;
  env.Initialize();
  DebugCallerChannelAccessor<Channel2::kDataSize> caller_ca([&env, send_buf]() {
      for(int i = 0; i < Channel2::kDataSize; i++) {
        // callee channel writes to the buffer.
        env._callee_ch->Write(i, (reinterpret_cast<const uint8_t *>(send_buf))[i]);
      }
      env._callee_ch->Return(0);
    }, *env._caller_ch, env._callee_id, GetDummySignal());
  caller_ca.Call();
  
  for(int i = 0; i < kBufSize; i++) {
    // read from the buffer.
    receive_buf[i] = caller_ca.Read(CallerChannelAccessor<Channel2::kDataSize>::Offset<int>(i * sizeof(int)));
  }
  MEMCMP_EQUAL(receive_buf, send_buf, Channel2::kDataSize);
}

TEST(CallerChannelAccessor, DoNotReadBeforeCall) {
  static const int kBufSize = Channel2::kDataSize / sizeof(int);
  int buf[kBufSize];
  
  CallerChannelAccessorEnv env;
  env.Initialize();
  DebugCallerChannelAccessor<Channel2::kDataSize> caller_ca([]() {
    }, *env._caller_ch, env._callee_id, GetDummySignal());

  CHECK_THROWS(AssertException, caller_ca.Read(CallerChannelAccessor<Channel2::kDataSize>::Offset<int>(0)));
  // caller_ca.Call();
}

TEST(CallerChannelAccessor, DoNotWriteAfterCall) {
  static const int kBufSize = Channel2::kDataSize / sizeof(int);
  int buf[kBufSize];
  
  CallerChannelAccessorEnv env;
  env.Initialize();
  DebugCallerChannelAccessor<Channel2::kDataSize> caller_ca([&env]() {
      env._callee_ch->Return(0);
    }, *env._caller_ch, env._callee_id, GetDummySignal());

  caller_ca.Call();
  CHECK_THROWS(AssertException, caller_ca.Write(CallerChannelAccessor<Channel2::kDataSize>::Offset<int>(0), 0));
}

TEST_GROUP(CalleeChannelAccessor) {
  TEST_SETUP() {
  }

  TEST_TEARDOWN() {
    mock().clear();
  }
};

TEST(CalleeChannelAccessor, Do) {
  class DebugCalleeChannelAccessor : public CalleeChannelAccessor<0> {
  public:
    DebugCalleeChannelAccessor(Channel2 &_ch) : CalleeChannelAccessor<0>(_ch) {
    }
    virtual void Do() override {
      mock().actualCall("Do");
    }
  };

  uint8_t *channel_buf = new uint8_t[Channel2::kBufAddress];
  Channel2::InitBuffer(channel_buf);
  Channel2 _ch(channel_buf, Channel2::Id::Null());
  DebugCalleeChannelAccessor ca(_ch);
  mock().expectOneCall("Do");
  ca.Call();
  mock().checkExpectations();
  delete [] channel_buf;
}

/*#include "CppUTest/TestHarness.h"
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

class ChildChannel : public Channel {
public:
  ChildChannel() : Channel(0, nullptr) {
  }
};

TEST(ChannelAccessor, Do) {
  const int16_t kType = rand();
  const int kReturnValue = rand();
  mock("Channel").expectOneCall("SendSignal").withParameter("type", kType).andReturnValue(kReturnValue);
  mock("Channel").ignoreOtherCalls();

  ChildChannel ch;
  ChannelAccessor<> ch_ac(ch, kType);
  CHECK_EQUAL(kReturnValue, ch_ac.Do(0));

  mock().checkExpectations();
}

TEST(ChannelAccessor, Id) {
  const int16_t kId = rand();
  mock("Channel").expectOneCall("Reserve").withParameter("id", kId);
  mock("Channel").ignoreOtherCalls();

  ChildChannel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Do(kId);

  mock().checkExpectations();
}

TEST(ChannelAccessor, ReadWrite0) {
  Debug::InitChannelBuffer();
  mock("Channel").ignoreOtherCalls();
  
  int16_t expected[Channel::kDataAreaSizeMax / sizeof(int16_t)];
  memset(expected, 0, sizeof(expected));

  ChildChannel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Do(0);

  MEMCMP_EQUAL(expected, Debug::channel_write_buffer, Channel::kDataAreaSizeMax);

  mock().checkExpectations();
}

TEST(ChannelAccessor, Write1) {
  Debug::InitChannelBuffer();
  mock("Channel").ignoreOtherCalls();
  
  const int offset = align<int>((rand() % Channel::kDataAreaSizeMax), sizeof(int16_t));
  int16_t data = rand() % Channel::kDataAreaSizeMax;
  int16_t expected[Channel::kDataAreaSizeMax / sizeof(int16_t)];
  static_assert(sizeof(expected) == Channel::kDataAreaSizeMax, "");
  memset(expected, 0, sizeof(expected));
  expected[offset / sizeof(int16_t)] = data;

  ChildChannel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Write<int16_t>(offset, data);
  ch_ac.Do(0);

  MEMCMP_EQUAL(expected, Debug::channel_write_buffer, Channel::kDataAreaSizeMax);

  mock().checkExpectations();
}

TEST(ChannelAccessor, WriteN) {
  Debug::InitChannelBuffer();
  mock("Channel").ignoreOtherCalls();

  int16_t expected[Channel::kDataAreaSizeMax / sizeof(int16_t)];
  memset(expected, 0, sizeof(expected));

  ChildChannel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  for (int i = 0; i < (rand() % Channel::kDataAreaSizeMax); i++) {
    const int offset = align<int>((rand() % Channel::kDataAreaSizeMax), sizeof(int16_t));
    int16_t data = rand() % Channel::kDataAreaSizeMax;
    expected[offset / sizeof(int16_t)] = data;
    ch_ac.Write<int16_t>(offset, data);
  }
  ch_ac.Do(0);

  MEMCMP_EQUAL(expected, Debug::channel_write_buffer, Channel::kDataAreaSizeMax);

  mock().checkExpectations();
}

TEST(ChannelAccessor, Read1) {
  Debug::InitChannelBuffer();
  mock("Channel").ignoreOtherCalls();
  
  const int offset = align<int>((rand() % Channel::kDataAreaSizeMax), sizeof(int16_t));

  ChildChannel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Do(0);
  CHECK_EQUAL(reinterpret_cast<int16_t *>(Debug::channel_read_buffer)[offset / sizeof(int16_t)], ch_ac.Read<int16_t>(offset));

  mock().checkExpectations();
}

TEST(ChannelAccessor, ReadN) {
  Debug::InitChannelBuffer();
  mock("Channel").ignoreOtherCalls();
  
  ChildChannel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Do(0);
  for (int i = 0; i < (rand() % Channel::kDataAreaSizeMax); i++) {
    const int offset = align<int>((rand() % Channel::kDataAreaSizeMax), sizeof(int16_t));
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
  
  const int offset = align<int>((rand() % Channel::kDataAreaSizeMax), sizeof(int16_t)) + 1;

  ChildChannel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Write<int16_t>(offset, 0);
  ch_ac.Do(0);

  mock().checkExpectations();
}

// Offset must be aligned.
TEST(ChannelAccessor, ReadAlign) {
  mock().expectOneCall("assert");
  mock("Channel").ignoreOtherCalls();
  
  const int offset = align<int>((rand() % Channel::kDataAreaSizeMax), sizeof(int16_t)) + 1;

  ChildChannel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Do(0);
  ch_ac.Read<int16_t>(offset);

  mock().checkExpectations();
}

// Write() must be called before Do()
TEST(ChannelAccessor, WriteAfterDo) {
  mock().expectOneCall("assert");
  mock("Channel").ignoreOtherCalls();
  
  ChildChannel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Do(0);
  ch_ac.Write<int16_t>(0, 0);

  mock().checkExpectations();
}

// Read() must be called after Do()
TEST(ChannelAccessor, ReadBeforeDo) {
  mock().expectOneCall("assert");
  mock("Channel").ignoreOtherCalls();
  
  ChildChannel ch;
  ChannelAccessor<> ch_ac(ch, 0);
  ch_ac.Read<int16_t>(0);
  ch_ac.Do(0);

  mock().checkExpectations();
}

// Offset must be smaller than kBufSize
TEST(ChannelAccessor, WriteOffset) {
  mock().expectOneCall("assert");
  mock("Channel").ignoreOtherCalls();
  
  const int offset = Channel::kDataAreaSizeMax;

  ChildChannel ch;
  ChannelAccessor<Channel::kDataAreaSizeMax - 1> ch_ac(ch, 0);
  ch_ac.Write<int16_t>(offset, 0);
  ch_ac.Do(0);

  mock().checkExpectations();
}

// Offset must be smaller than kBufSize
TEST(ChannelAccessor, ReadOffset) {
  mock().expectOneCall("assert");
  mock("Channel").ignoreOtherCalls();
  
  const int offset = Channel::kDataAreaSizeMax;

  ChildChannel ch;
  ChannelAccessor<Channel::kDataAreaSizeMax - 1> ch_ac(ch, 0);
  ch_ac.Do(0);
  ch_ac.Read<int16_t>(offset);

  mock().checkExpectations();
}
*/
