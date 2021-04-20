#ifndef TIMER_H
#define TIMER_H
#include <Arduino.h>

class Timer
{
  static constexpr unsigned long MAX_ULONG{4294967295};
  unsigned long previousMillis;
  unsigned long currentTime;
  
public:
  Timer() = default;
  ~Timer() {}

  // returns time since last reset in seconds
  unsigned long getTime();
  void reset();
};

#endif
