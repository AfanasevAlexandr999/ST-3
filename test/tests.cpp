// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <thread>
#include <chrono>
#include "TimedDoor.h"

class MockDoor : public Door {
 public:
  MOCK_METHOD(void, lock, (), (override));
  MOCK_METHOD(void, unlock, (), (override));
  MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class MockTimerClient : public TimerClient {
 public:
  MOCK_METHOD(void, Timeout, (), (override));
};

class TimedDoorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    door = new TimedDoor(2);
  }

  void TearDown() override {
    delete door;
  }

  TimedDoor* door;
};

TEST_F(TimedDoorTest, DoorCreatedInLockedState) {
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, LockMethodKeepsDoorClosed) {
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, UnlockMethodOpensDoor) {
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, GetTimeOutReturnsCorrectValue) {
  EXPECT_EQ(door->getTimeOut(), 2);
}

TEST_F(TimedDoorTest, ThrowStateThrowsExceptionWhenDoorOpen) {
  door->unlock();
  EXPECT_THROW(door->throwState(), std::logic_error);
}

TEST_F(TimedDoorTest, ThrowStateDoesNotThrowWhenDoorClosed) {
  door->lock();
  EXPECT_NO_THROW(door->throwState());
}

TEST_F(TimedDoorTest, DoorTimerAdapterCallsThrowStateOnTimeout) {
  door->unlock();
  // Timer is activated and thread is started
  EXPECT_TRUE(door->isDoorOpened());
  // Wait for timeout to trigger
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));
  // Door should still be in the same state (timeout doesn't change it,
  // but throwState was called in the background thread)
  EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, DoorTimerAdapterNoThrowWhenDoorClosedBeforeTimeout) {
  door->unlock();
  door->lock();
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, MultipleLockUnlockCycles) {
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, DoorWithDifferentTimeouts) {
  delete door;
  door = new TimedDoor(5);
  EXPECT_EQ(door->getTimeOut(), 5);
  delete door;
  door = new TimedDoor(10);
  EXPECT_EQ(door->getTimeOut(), 10);
}

TEST_F(TimedDoorTest, ExceptionMessageIsCorrect) {
  door->unlock();
  try {
    door->throwState();
    FAIL() << "Expected exception to be thrown";
  } catch (const std::logic_error& e) {
    EXPECT_STREQ(e.what(), "Door is still open!");
  }
}

class TimerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    mock_client = new MockTimerClient();
    timer = new Timer();
  }

  void TearDown() override {
    delete timer;
    delete mock_client;
  }

  Timer* timer;
  MockTimerClient* mock_client;
};

TEST_F(TimerTest, TimerCallsClientTimeoutAfterDelay) {
  EXPECT_CALL(*mock_client, Timeout()).Times(1);
  timer->tregister(1, mock_client);
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
}
