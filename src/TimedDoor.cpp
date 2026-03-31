// Copyright 2021 GHA Test Team
#include "TimedDoor.h"
#include <thread>
#include <chrono>
#include <stdexcept>

Timer::Timer() : client(nullptr) {}

void Timer::sleep(int seconds) {
  std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void Timer::tregister(int timeout, TimerClient* timerClient) {
  client = timerClient;
  std::thread([timerClient, timeout]() {
    std::this_thread::sleep_for(std::chrono::seconds(timeout));
    if (timerClient != nullptr) {
      try {
        timerClient->Timeout();
      } catch (...) {
        // Exception caught and handled by TimerClient
      }
    }
  }).detach();
}

TimedDoor::TimedDoor(int timeout)
    : iTimeout(timeout), isOpened(false) {
  adapter = new DoorTimerAdapter(*this);
}

TimedDoor::~TimedDoor() {
  delete adapter;
}

void TimedDoor::lock() {
  isOpened = false;
}

void TimedDoor::unlock() {
  isOpened = true;
  Timer timer;
  timer.tregister(iTimeout, adapter);
}

bool TimedDoor::isDoorOpened() {
  return isOpened;
}

int TimedDoor::getTimeOut() const {
  return iTimeout;
}

void TimedDoor::throwState() {
  if (isOpened) {
    throw std::logic_error("Door is still open!");
  }
}

DoorTimerAdapter::DoorTimerAdapter(TimedDoor& timedDoor)
    : door(timedDoor) {}

void DoorTimerAdapter::Timeout() {
  door.throwState();
}
