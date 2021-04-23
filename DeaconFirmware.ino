#include <NeoSWSerial.h>
#include "Timer.h"
#include "millisDelay.h"
// OK+DISISOK+DISC:4C000215:2G234454CF6D5A0FADF2F4911BA9FFA7:00000001AC:0CF3EE041CCE:-052OK+DISC:00000000:00000000000000000000000000000000:0000000000:B9782E08068C:-071OK+DISCE

struct Reading
{
  unsigned long TS;
  String UUID;
  String RSSI;
};

NeoSWSerial bt{ 2, 3 };
Timer timer{};
millisDelay loopDelay;

int numReadings{ 10 };  //
int scanInterval{ 100 }; // time in ms between scans.

String readLineFromSD(int i);


void setupBT();

// returns result if success. returns blank string if failure.
String scan();
void writeReading(const Reading& reading); 


void setup()
{
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
  bt.begin(9600);
  loopDelay.start(50);
  setupBT();
}

void loop()
{
  if (loopDelay.justFinished())
  {
    String rawData{ scan() };

    // iterates over string looking for '-' characters, and assigns the index of
    // these in the string to i.
    for(int i{rawData.indexOf('-')};
      i < rawData.length();
      i = rawData.indexOf('-', i + 1))
    {
      Reading reading;
      reading.TS = timer.getTime();
      reading.UUID = rawData.substring(i /* + something*/, i /* + something*/);
      reading.RSSI = rawData.substring(i /* + something*/, i /* + something*/);
      writeReading(reading);
    }
    
    loopDelay.start(scanInterval);
  }
}

// Called whenever arduino recieves something from serial port.
// Will not call when loop() is running, so don't block there.
void serialEvent()
{
  String msg{ Serial.readStringUntil('\n') };

  if (msg == "sync")
  {
    // write timestamp
    Serial.write("ts:");
    Serial.write(String(timer.getTime()).c_str());
    Serial.write('\n');

    // write all readings
    for (int i{}; i < numReadings; ++i)
      Serial.write(readLineFromSD(i).c_str());

    // notify completion and reset timer
    Serial.write("done\n");
    timer.reset();
  }
  else
  {
    Serial.write("Unrecognized message\n");
  }
}

String scan()
{
  millisDelay timeout;
  bt.write("AT+DISI?");
  timeout.start(5000);
  // loop runs given that the 5 second timout hasn't finished,
  // in which case it gives up and returns an empty string
  while(!timeout.justFinished())
    if(bt.available())
      return bt.readStringUntil('\n');
  return "";
}

void setupBT()
{
  // this needs to setup the bluetooth module
  // as a beacon and as something that can listen for beacons.
}

void writeReading(const Reading& reading)
{
  // this needs to write a line to the sd card with each part of reading
}

String readLineFromSD(int i)
{
  // this needs to read a line from the sd card, with a newline character at the end.
  return "FFFFFFFF,db559330acb9442398e3248894e62ed1,069\n";
}
