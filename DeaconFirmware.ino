#include <NeoSWSerial.h>
#include "Timer.h"

NeoSWSerial bluetoothModule{ 2, 3 };
Timer timer{};

// currently arbitrary number for testing
int numReadings{ 10 };

// current format: 'timestamp,uuid,rssi'
String readLineFromSD(int i);

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  Serial.write(String(timer.getTime()).c_str());
  Serial.write('\n');
  delay(1000);
}

// Called whenever arduino recieves something from serial port.
// Will not call when loop() is running, so don't block there.
void serialEvent()
{
  String msg{ Serial.readStringUntil('\n') };
  
  //if(!msg)
  // return;
  
  if(msg == "sync")
  {
    // write timestamp
    Serial.write("ts:");
    Serial.write(String(timer.getTime()).c_str());
    Serial.write('\n');

    // write all readings
    for(int i{}; i < numReadings; ++i)
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

String readLineFromSD(int i)
{
  return "FFFFFFFF,db559330acb9442398e3248894e62ed1,069\n";
}
