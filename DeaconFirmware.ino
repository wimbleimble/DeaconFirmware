#include <NeoSWSerial.h>
#include <SPI.h>
#include <SD.h>
#include "millisDelay.h"
#include "Timer.h"

namespace sd
{
  void init();
  void writeReading(unsigned long timestamp, const String& uuid, const String& rssi);
  String readLine(int line);
  void wipe();
}

String scan();

NeoSWSerial bt{ 2, 3 };
Timer timer{};
millisDelay loopDelay;

// you could probably do this without keeping track of this, but honestly this is easiest
constexpr int numReadings{ 5 };
constexpr int scanInterval{ 100 };
constexpr char UUID[]{ "ac5fe330acb93642398e7348894e62ed" };

void setup()
{
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
  bt.begin(9600);
  sd::init();
  loopDelay.start(50);
}

void loop()
{
  if (loopDelay.justFinished())
  {
//    String rawData{ scan() };
//
//    // iterates over string looking for '-' characters, and assigns the index of
//    // these in the string to i.
//    for(int i{rawData.indexOf('-')};
//      i < rawData.length();
//      i = rawData.indexOf('-', i + 1))
//    {
//      if(i >= 57) // prevents out of bounds index
//      {
//        ++numReadings;
//        sd::writeReading(
//          timer.getTime(),
//          rawData.substring(i - 57, i - 26),
//          rawData.substring(i + 1, i + 3)
//        );
//      }
//    }
//    
    loopDelay.start(scanInterval);
  }
}

String scan()
{
  millisDelay timeout;
  bt.write("AT+DISI?");
  timeout.start(5000);
  // pretty sure this doesn't work. hard to figure out when 
  // we keep getting a heap/stack collision tho
  while(!timeout.justFinished())
    if(bt.available())
      return bt.readStringUntil('\n');
  return "";
}

void sd::init()
{
  if (!SD.begin(10))
  {
    Serial.println("Card failed, or not present");
    while (1);
  }
}

void sd::writeReading(unsigned long timestamp, const String& uuid, const String& rssi)
{
  
  File file{ SD.open("data.csv", FILE_WRITE) };

  // if the file is available, write to it:
  if (file)
  {
    file.print(timestamp);
    file.print(',');
    file.print(uuid);
    file.print(',');
    file.print(rssi);
    file.print('\n');
    file.close();
  }
}

String sd::readLine(int line)
{
  File file{ SD.open("data.csv") };
  String ret{};

  for(int i{ 0 }; i < line; ++i)
  {
    if(file.available())
      while(file.read() != '\n');
    else
      return "";
  }    
  if(file)
    while(!ret.endsWith("\n") && file.available())
      ret += (char)file.read();
  file.close();
  return ret;
}

void sd::wipe()
{
  SD.remove("data.csv");
}

void serialEvent()
{
  String msg{ Serial.readStringUntil('\n') };

  if (msg == "sync")
  {
    // write timestamp
    Serial.write("ts:");
    Serial.write(String(timer.getTime()).c_str());
    Serial.write('\n');
    
    // write beacon's uuid
    Serial.write("uuid:");
    Serial.write(UUID);
    Serial.write('\n');

    // write all readings
    for (int i{}; i < numReadings; ++i)
      Serial.write(sd::readLine(i).c_str());

    // notify completion and reset timer
    Serial.write("done\n");
    timer.reset();
    // sd::wipe();
  }
  else if (msg == "uuid")
  {
    Serial.write("uuid:");
    Serial.write(UUID);
    Serial.write('\n');
  }
  else
  {
    Serial.write("Unrecognized message\n");
  }
}
