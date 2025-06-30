/*
The sensor outputs provided by the library are the raw
16-bit values obtained by concatenating the 8-bit high and
low accelerometer and gyro data registers. They can be
converted to units of g and dps (degrees per second) using
the conversion factors specified in the datasheet for your
particular device and full scale setting (gain).

Example: An LSM6DS33 gives an accelerometer Z axis reading
of 16276 with its default full scale setting of +/- 2 g. The
LA_So specification in the LSM6DS33 datasheet (page 11)
states a conversion factor of 0.061 mg/LSB (least
significant bit) at this FS setting, so the raw reading of
16276 corresponds to 16276 * 0.061 = 992.8 mg = 0.9928 g.
*/

const byte ENABLE_USB_MIDI = 0;
const byte SERIAL_DEBUG = 1;

#include <Wire.h>
#include "USB_MIDI.h"
#include "buttons.h"
#include "controller.h"
#include "LSM6.h"

#include <FastLED.h>

LSM6 imu;

char report[80];

Button buttons[] = {Button(7), Button(8), Button(9), Button(10), Button(0)}; //pin x is the boot button
 
//(int ccNumber, int minInterval = 20, int inLow = 0, int inHigh = 127, int deltaThreshold = 2, float alpha = 0.2))
CController cc[] = {
  CController(0, 50, 1900, 3550, 3, .5), //hall1
  CController(1, 50, 1900, 3550, 3, .5), //hall2
  CController(2, 100, 100, 3900, 10, .4), //pot1
  CController(3, 100, 100, 3900, 10, .4), //pot2
  CController(4, 100, 20, 400, 10, .4),  //Capsense1
  CController(5, 100, 20, 400, 10, .4), //Capsense2

  CController(10, 50, 0, 2000, 10, .2),
  CController(11, 50, 0, 2000, 10, .2),
  CController(12, 50, 0, 2000, 10, .2),
  CController(13, 50, 0, 2000, 10, .2),
  CController(14, 50, 0, 2000, 10, .2),
  CController(15, 50, 0, 2000, 10, .2),
  CController(16, 50, 0, 2000, 10, .2),
  CController(17, 50, 0, 2000, 10, .2),
  CController(18, 50, 0, 2000, 10, .2),
  CController(19, 50, 0, 2000, 10, .2),
  CController(20, 50, 0, 2000, 10, .2),
  CController(21, 50, 0, 2000, 10, .2)
};

void setup()
{
  if( ENABLE_USB_MIDI) usbMidiSetup();
  else Serial.begin(115200);
  Wire.begin(6,5);

  if (!imu.init())
  {
    Serial.println("Failed to detect and initialize IMU!");
    while (1);
  }
  imu.enableDefault();

  if( ENABLE_USB_MIDI != 1) Serial.println("USB MIDI not enabled");
}

void loop()
{
  // // Handle MIDI input
  static uint32_t timer = 0;
  int interval = 100; 

  if(millis()-timer > interval){
    timer= millis();

      imu.read();

      if(1){
        snprintf(report, sizeof(report), "A: %6d %6d %6d    G: %6d %6d %6d",
        imu.a.x, imu.a.y, imu.a.z,
        imu.g.x, imu.g.y, imu.g.z);
        Serial.println(report);
        Serial.print(analogRead(1));
        Serial.print("\t");
        Serial.print(analogRead(2));
        Serial.print("\t");
        Serial.print(digitalRead(7));
        Serial.print("\t");
        Serial.print(digitalRead(8));
        Serial.print("\t");
        Serial.print(digitalRead(9));
        Serial.print("\t");
        Serial.print(digitalRead(10));
        Serial.print("\t");
        Serial.print(digitalRead(0));
        Serial.println();
      }

      else{

        //cc[0].send(analogRead(12));
        cc[0].send(analogRead(1)); //12    2
        cc[1].send(analogRead(2)); // 5     9
        cc[2].send(digitalRead(7)); //11    3
        cc[3].send(digitalRead(8)); //10    4
        cc[4].send(digitalRead(9)); //4       10
        cc[5].send(digitalRead(10)); //7       7
      }

  }
}