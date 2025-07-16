/*
Wand v2
July 13, 2025
Final wand firmware for Creativitas Summer 2025

*/

const byte ENABLE_USB_MIDI = 0;
const byte SERIAL_DEBUG = 1;

#include <Wire.h>
#include "USB_MIDI.h"
#include "buttons.h"
#include "controller.h"
#include "LSM6.h"
#include "sensor.h"

#include <FastLED.h>
#define NUM_LEDS 10
#define LED_PIN 4
CRGB leds[NUM_LEDS];
CRGB built_in[1]; // Array for LEDs on pin 21
CHSV tipColor = CHSV(160, 255, 255); // Aqua hue
CHSV curColor = CHSV(0,0,0);

LSM6 imu;

float accelX, accelY, accelZ; // Replace with real IMU readings
float accelRest = 0;
float flickThreshold = 1;   // Gs or unit matching your IMU
uint8_t flashBrightness = 0;
// Sensitivity settings (adjust for your IMU and preference)
const float FLICK_THRESHOLD = 0.5; // in g
const uint8_t DECAY_RATE = 20;      // how fast to fade (higher = faster)
uint8_t buttonState[5];
uint8_t buttonIsPressed = 0;

// State variable
uint8_t gradientBrightness = 0; // Current max brightness of tip

uint8_t motionLevel = 0; // 0–255
const uint8_t LEAK_AMOUNT = 25;   // how fast it fades (1–10)
const uint8_t INPUT_GAIN = 255; // scale input motion (adjust as needed)
const float MOTION_SCALE = 1.0; // scale motionMagnitude into 0–1

char report[80];

//IMU DATA PROCESSING

SensorTriple accel;
SensorTriple gyro;

struct SmoothedIMU {
  float accelX, accelY, accelZ;
  float gyroX, gyroY, gyroZ;
};
SmoothedIMU smoothedIMU = {0};
const float smoothingAlpha = 0.1;  // Lower = more smoothing

struct TiltAngles {
  float roll;   // degrees
  float pitch;  // degrees
  float yaw;    // degrees (approximate from tilt)
};

struct GyroAngle {
  float x = 0;
  float y = 0;
  float z = 0;
};

GyroAngle integratedGyro;
const float decayFactor = 0.995;  // Closer to 1 = less leak
unsigned long lastTime = 0;

Button buttons[] = {Button(7), Button(8), Button(9), Button(10), Button(0)}; //pin x is the boot button
 
//(int ccNumber, int minInterval = 20, int inLow = 0, int inHigh = 127, int deltaThreshold = 2, float alpha = 0.2))
CController cc[] = {
  CController(0, 50, 10, 4065, 5,   .001, 0.1, 4), //pot1
  CController(1, 50, 10, 4065, 5,   .001, 0.1, 4), //pot2
  CController(2, 20, -100, 100, 5,   .001, 0.01, 4), //magnitude
  CController(3, 20, -180, 180, 5,   .001, 0.01, 4), //tilt
  CController(4, 20, -180, 180, 5,   .001, 0.01, 4),  //roll
  CController(5, 20, -180, 180, 5,   .001, 0.01, 4), //yaw
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
  delay(100);
  imu.read();
  accelRest = calcMagnitude();

  if( ENABLE_USB_MIDI != 1) Serial.println("USB MIDI not enabled");

  FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, NUM_LEDS);//.setCorrection( TypicalLEDStrip );
  FastLED.addLeds<WS2811, 21, GRB>(built_in, 1);
  FastLED.clear();
  built_in[0] = CRGB(0, 0, 20);
  FastLED.show();
  FastLED.setBrightness(255);  // Max brightness
}

void loop()
{
  readButtons();

  static uint32_t timer = 0;
  int interval = 20; 

  if(millis()-timer > interval){
    timer= millis();

    statusLed(0);
    imu.read(); 
    accel.update(imu.a.x, imu.a.y, imu.a.z);
    gyro.update(imu.g.x, imu.g.y, imu.g.z);

    float magnitude = calcMagnitude();
    TiltAngles angles = computeTiltAngles(imu.a.x, imu.a.y, imu.a.z);
    // updateSmoothedIMU(imu.a.x, imu.a.y, imu.a.z, imu.g.x, imu.g.y, imu.g.z);
    // updateIntegratedGyro(imu.g.x, imu.g.y, imu.g.z);
    // updateIntegratedGyro(imu.g.x, imu.g.y, imu.g.z);
    calcDrumStrike(magnitude);

    updateLeakyGradient(magnitude, getBaseColorFromButtons());


    cc[0].send(analogRead(1));
    cc[1].send(analogRead(2));
    //if(buttonIsPressed == 0) return

   // cc[2].send(int(magnitude * 100));

    
    // cc[3].send(angles.roll);
    // cc[4].send(angles.pitch);
    // cc[5].send(angles.yaw); 
    Serial.print(accel.x.getScaledSmoothed());
    Serial.print(",");
    Serial.print(accel.y.getScaledSmoothed());
    Serial.print(",");
    Serial.print(accel.z.getScaledSmoothed());
    Serial.println(",");

    // cc[3].send(accel.x.getScaledSmoothed());
    // cc[4].send(accel.y.getScaledSmoothed());
    // cc[5].send(accel.z.getScaledSmoothed());
    // cc[30].send(smoothedIMU.gyroX);
    // cc[31].send(smoothedIMU.gyroY);
    // cc[32].send(smoothedIMU.gyroZ);

    // cc[40].send(integratedGyro.x);
    // cc[41].send(integratedGyro.y);
    // cc[42].send(integratedGyro.z);
  }
} // LOOP

void readButtons(){
  byte midiNotes[] = {60,62,64,65,72};
  buttonIsPressed = 0;

  for(int i=0;i<5;i++){
    buttons[i].loop();
    if(buttons[i].isPressed()){
      sendButtonPress(midiNotes[i], 127);
      buttonIsPressed = 1;
    } else if( buttons[i].isReleased()){
      sendButtonPress(midiNotes[i], 0);
    } else if( buttons[i].isDown()) {
      buttonIsPressed = 1;
    }
  }

}

float calcMagnitude(){
  // sensitivity = 0.000061 for ±2g (default)
  // use 0.000122 for ±4g, etc.
  const float ACCEL_SENSITIVITY = 0.000061;

  float ax_g = imu.a.x * ACCEL_SENSITIVITY;
  float ay_g = imu.a.y * ACCEL_SENSITIVITY;
  float az_g = imu.a.z * ACCEL_SENSITIVITY;

  return sqrt(ax_g * ax_g + ay_g * ay_g + az_g * az_g) - accelRest;
}


void updateLeakyGradient(float motionMagnitude, CRGB baseColor) {
  static uint8_t progress = 0; // cycles from 0–9
  const uint8_t blendAmount = 200;  // 0–255. Higher = more trail blending
  const float waveSpeed = 0.1;      // Adjust this for wave contrast

  // --- Leaky integrator logic ---
  motionLevel = qsub8(motionLevel, LEAK_AMOUNT);

  uint8_t inputValue = constrain(motionMagnitude * INPUT_GAIN * MOTION_SCALE, 0, 255);
  motionLevel = qadd8(motionLevel, inputValue);

  // --- Color trail with smoothing and wave modulation ---
  for (int i = 0; i < NUM_LEDS; i++) {
    // Brightness scaling based on distance from tip and motion level
    uint8_t scale = map(i, 0, NUM_LEDS - 1, 0.2, motionLevel);

    // LED 0 gets base color blended slightly with black
    if (i == 0) {
      leds[0] = baseColor; //blend(baseColor, CRGB::Black, blendAmount);
    } else {
      leds[i] = blend(baseColor, leds[i - 1], blendAmount);
    }

    // Optional: subtle wave pattern modulation across the strip
    float wave = fabsf((float)((i % 10) - progress)) / 10.0 + 0.5;
    leds[i].nscale8_video((uint8_t)(180 * waveSpeed * wave + 75)); // Wave dimming

    // Apply brightness scale from motion level
    leds[i].nscale8_video(scale);
  }

  progress = (progress + 1) % 10;
  FastLED.show();
}

CRGB getBaseColorFromButtons() {
  CRGB color = CRGB::Black;

  if (digitalRead(7) == LOW) color += CRGB(255, 100, 0); //orange
  if (digitalRead(8) == LOW) color += CRGB(180, 0, 255); // Purple
  if (digitalRead(9) == LOW) color += CRGB(0, 200, 255); // turquoise
  if (digitalRead(10) == LOW) color += CRGB(100, 255, 100); // ???

  // Clamp to max brightness
  color.red   = qadd8(color.red,   0); // clamps to 255 if already too high
  color.green = qadd8(color.green, 0);
  color.blue  = qadd8(color.blue,  0);

  return color;
}

void sendButtonPress(byte note, byte velocity){
  if( SERIAL_DEBUG ) {
    Serial.print("midi note \t ");
    Serial.print(note);
    Serial.print("\t ");
    Serial.println(velocity);
  } else{
    sendMidiNoteOn(note, velocity);
  }
}

// accelX, accelY, accelZ must be in Gs
TiltAngles computeTiltAngles(float accelX, float accelY, float accelZ) {
  TiltAngles angles;

  angles.roll = atan2(accelY, accelZ) * 180.0 / PI * 1;
  angles.pitch = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0 / PI * 1;
  angles.yaw = atan2(accelY, accelX) * 180.0 / PI * 1;

  angles.roll = abs( angles.roll );
  angles.pitch = abs( angles.pitch );
  angles.yaw = abs( angles.yaw );

  return angles;
}

void updateSmoothedIMU(float ax, float ay, float az, float gx, float gy, float gz) {
  smoothedIMU.accelX = smoothingAlpha * ax + (1 - smoothingAlpha) * smoothedIMU.accelX;
  smoothedIMU.accelY = smoothingAlpha * ay + (1 - smoothingAlpha) * smoothedIMU.accelY;
  smoothedIMU.accelZ = smoothingAlpha * az + (1 - smoothingAlpha) * smoothedIMU.accelZ;

  smoothedIMU.gyroX = smoothingAlpha * gx + (1 - smoothingAlpha) * smoothedIMU.gyroX;
  smoothedIMU.gyroY = smoothingAlpha * gy + (1 - smoothingAlpha) * smoothedIMU.gyroY;
  smoothedIMU.gyroZ = smoothingAlpha * gz + (1 - smoothingAlpha) * smoothedIMU.gyroZ;
}

void updateIntegratedGyro(float gx, float gy, float gz) {
  unsigned long now = millis();
  float dt = (lastTime > 0) ? (now - lastTime) / 1000.0 : 0.01;
  lastTime = now;

  // Integrate and apply decay
  integratedGyro.x = (integratedGyro.x + gx * dt) * decayFactor;
  integratedGyro.y = (integratedGyro.y + gy * dt) * decayFactor;
  integratedGyro.z = (integratedGyro.z + gz * dt) * decayFactor;
}

void calcDrumStrike(float magnitude){
  static float prevMagnitude = 0;
  float delta = magnitude - prevMagnitude;
  static bool triggered = false;

   static float magnitudeBuffer[4];
  static uint8_t magIndex = 0;
    magnitudeBuffer[magIndex] = delta;
    magIndex = (magIndex+1) % 4;

  const float triggerThreshold = 1.0;
  const float resetThreshold = 0.1;

  static unsigned long lastTriggerTime = 0;
  unsigned long now = millis();

  if (!triggered && delta > triggerThreshold) {
    // Rising edge — trigger strike
    triggered = true;

    float peakMagnitude = 0;
    for(byte i=0; i<4; i++) peakMagnitude += magnitudeBuffer[ magIndex];
    uint8_t velocity = map(peakMagnitude * 127, 100, 300, 20, 127);
    velocity = constrain( velocity, 60,127);
    if( millis() - lastTriggerTime > 5) sendButtonPress(36, velocity);
    //usbMIDI.noteOn(36, velocity);
    //usbMIDI.noteOff(60); // optional immediate off or schedule later

    lastTriggerTime = now;
  }

  if (triggered && magnitude < resetThreshold) {
    // Falling edge — ready to trigger again
    triggered = false;
    sendButtonPress(36, 0);
  }
}

/******
MIDI INPUT
******/
void handleControlChange(uint8_t channel, uint8_t number, uint8_t value) {
  statusLed(1);
  // if( number == 127) {monitorInput = -1; return;} //disable all monitoring
  // else if( number >=80 && number<100){ monitorInput = number - 80; return;} //monitor raw data
  // else if( number >=60 && number<80){ cap[number-60].upperThreshold = value*20; return;} //update high threshold
  // else if( number >=40 && number<60){ cap[number-40].lowerThreshold = value*20;} //update low threshold
  // else if( number >=20 && number<40){ cap[number-20].upperThreshold = value*20;} //update velocity depth
  //else if( number >=0 && number<20){ monitorInput = number - 80; return;}
  // 
  // if (number == 120 && value == 120) { // CC 120 triggers the parameter query
  //       sendStoredParameters();
  //       return;
  //   } else updateParameterValue(channel, number, value);
}

void statusLed(int num){
  byte color[][3] = {
    {100, 0, 0},   // Red
    {0, 100, 0},   // Green
    {0, 0, 100},   // Blue
    {100, 0, 100}, // Magenta
    {100, 100, 100} // White
  };
  static int state = 0;
  static uint32_t timer = 0;
  int interval = 63;
  if( num==0){
    if(state>0){
      if(millis()-timer > interval){
        state = 0;
        built_in[0] = CRGB(0, 0, 0);
        FastLED.show();
      }
      return;
    } else return;
  } 

  if(num>5) return;

  //flash led
  timer = millis();
  state=1;
  built_in[0] = CRGB(color[num-1][0], color[num-1][1], color[num-1][2]);
  FastLED.show();
}//status LED