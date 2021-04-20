#include "Timer.h"

unsigned long Timer::getTime()
{
  unsigned long currentMillis{ millis() };

  // if true, then there has been an overflow. this assumes one.
  // if, somehow, there's been >1 overflow, this function doesn't function.
  if(currentMillis < previousMillis)
    currentTime += (((MAX_ULONG - previousMillis) + currentMillis) / 1000);
  else
    currentTime += ((currentMillis - previousMillis) / 1000);
  
  previousMillis = currentMillis;

  return currentTime;
}

void Timer::reset()
{
  currentTime = 0;
}
