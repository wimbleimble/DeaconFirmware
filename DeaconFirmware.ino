#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include "millisDelay.h"
#include "Timer.h"

// compile time global constants
constexpr int scanInterval{ 100 };
constexpr int btTimeoutTime{ 8000 };
constexpr char UUID[] { "ac5fe330acb93642398e7348894e62ed" };

// types
enum class State
{
  WAIT,
  GO
};

// forwared delcarations

void waitState();
void goState();
void sync();

namespace sd
{
void init();
void writeReading(unsigned long timestamp, const String& uuid, const String& rssi);
String readLine(int line);
void wipe();
}

// non-const globals
SoftwareSerial bt{ 2, 3 };
Timer timer{};
millisDelay loopDelay;
millisDelay btTimeout;
State state{ State::WAIT };
String btBuffer{};
int numReadings{ 0 };

void setup()
{
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
  bt.begin(9600);
  sd::init();
  checkBT();
  loopDelay.start(5000);
}

void loop()
{
  switch (state)
  {
    case State::WAIT:
      waitState();
      break;
    case State::GO:
      goState();
      break;
  }
}

void checkBT()
{
  String buff{};
  while (true)
  {
    if (bt.available() > 0)
    {
      buff += (char)bt.read();
      buff += (char)bt.read();
      if (buff == "OK")
        return;
    }
    bt.write("AT");
    delay(100);
  }
}

void waitState()
{
  if (loopDelay.justFinished())
  {
    state = State::GO;
    bt.write("AT+DISI?");
    btTimeout.start(btTimeoutTime);

  }
  else if (Serial.available() > 0)
    sync(); 
}

void goState()
{
  if (bt.available() > 0)
  {
    char what( bt.read() );
    btBuffer += what;
    if (btBuffer.indexOf("OK+DISCE") > 0)
    {
      processBuffer();
      state = State::WAIT;
      loopDelay.start(scanInterval);
    }
  }
  else if(btTimeout.justFinished())
  {
    state = State::WAIT;
    btBuffer = "";
    loopDelay.start(scanInterval);
    bt.flush();
  }
}

void processBuffer()
{
  
  for (int i{btBuffer.indexOf('-')};
       i < btBuffer.length();
       i = btBuffer.indexOf('-', i + 1))
  {
    if (i >= 57) // prevents out of bounds index
    {
      if (btBuffer.substring(i - 24, i - 16) == "01A40045")
      {
        ++numReadings;
        sd::writeReading(
          timer.getTime(),
          btBuffer.substring(i - 57, i - 25),
          btBuffer.substring(i + 1, i + 4)
        );
      }
    }
  }
  btBuffer = "";
}

void sd::init()
{
  if (!SD.begin(10))
  {
    Serial.write("Failed to initialise SD.\n");
    while (true);
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

  for (int i{ 0 }; i < line; ++i)
  {
    if (file.available())
      while (file.read() != '\n');
    else
      return "";
  }
  if (file)
    while (!ret.endsWith("\n") && file.available())
      ret += (char)file.read();
  file.close();
  return ret;
}

void sd::wipe()
{
  SD.remove("data.csv");
}

void sync()
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
    sd::wipe();
  }
  else if (msg == "uuid")
  {
    Serial.write("uuid:");
    Serial.write(UUID);
    Serial.write('\n');
  }
  else
    Serial.write("Unrecognized message\n");
}
