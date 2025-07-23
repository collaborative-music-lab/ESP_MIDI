/*
Wand v2
July 13, 2025
Final wand firmware for Creativitas Summer 2025
*/

const byte ENABLE_USB_MIDI = 1;
const byte SERIAL_DEBUG = 0;

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
float tiltScaling = 1;
float magnitudeScaling = 1;

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

  CController(10, 20, 0, 180, 5,   .001, 0.01, 4), //3 tilt 10
  CController(11, 20, 0, 180, 5,   .001, 0.01, 4),  //roll
  CController(12, 20, 0, 180, 5,   .001, 0.01, 4), //yaw

  CController(20, 20, -20000, 20000, 5,   .1, 0.01, 4), //6 accel 20
  CController(21, 20, -20000, 20000, 5,   .1, 0.01, 4),  //
  CController(22, 20, -20000, 20000, 5,   .1, 0.01, 4), //

  CController(30, 20, -20000, 20000, 5,   .1, 0.01, 4), //9 gyro 30
  CController(31, 20, -20000, 20000, 5,   .1, 0.01, 4),  //
  CController(32, 20, -20000, 20000, 5,   .1, 0.01, 4), //

  CController(40, 50, -10000, 10000, 5,   .1, 0.01, 4), //12 angle 40
  CController(41, 50, -10000, 10000, 5,   .1, 0.01, 4),  //
  CController(42, 50, -10000, 10000, 5,   .1, 0.01, 4), //
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
  built_in[0] = CRGB(0, 20, 0);
  FastLED.show();
  FastLED.setBrightness(255);  // Max brightness
}

void loop()
{
  readButtons();
  processIncomingMidi();

  static uint32_t timer = 0;
  int interval = 5; 

  if(millis()-timer > interval){
    timer= millis();

    statusLed(0);
    imu.read(); 
    accel.update(imu.a.x, imu.a.y, imu.a.z);
    gyro.update(imu.g.x, imu.g.y, imu.g.z);

    float magnitude = calcMagnitude();
    TiltAngles angles = computeTiltAngles(imu.a.x, imu.a.y, imu.a.z);
    calcDrumStrike(magnitude);

    updateLeakyGradient(magnitude, getBaseColorFromButtons());


    cc[0].send(analogRead(1));
    cc[1].send(analogRead(2));
    if(buttonIsPressed == 0) return

    cc[2].send(int(magnitude * 100));

    if( tiltScaling > 0.01 ) cc[3].send(angles.roll * tiltScaling);
    if( tiltScaling > 0.01 ) cc[4].send(angles.pitch * tiltScaling);
    if( tiltScaling > 0.01 ) cc[5].send(angles.yaw * tiltScaling); 

    if( accel.x.rawScale > 0.01 ) cc[6].send(accel.x.getScaledSmoothed());
    if( accel.y.rawScale > 0.01 ) cc[7].send(accel.y.getScaledSmoothed());
    if( accel.z.rawScale > 0.01 ) cc[8].send(accel.z.getScaledSmoothed());

    if( gyro.x.rawScale > 0.01 )  cc[9].send(gyro.x.getScaledSmoothed());
    if( gyro.x.rawScale > 0.01 ) cc[10].send(gyro.y.getScaledSmoothed());
    if( gyro.x.rawScale > 0.01 ) cc[11].send(gyro.z.getScaledSmoothed());

    if( gyro.x.integratedScale > 0.01 )  cc[12].send(gyro.x.getScaledIntegrated());
    if( gyro.x.integratedScale > 0.01 ) cc[13].send(gyro.y.getScaledIntegrated());
    if( gyro.x.integratedScale > 0.01 ) cc[14].send(gyro.z.getScaledIntegrated());
  }
} // LOOP

void readButtons(){
  byte midiNotes[] = {60,62,64,65,72};
  buttonIsPressed = 0;

  for(int i=0;i<5;i++){
    buttons[i].loop();
    if(buttons[i].isPressed()){
      gyro.resetIntegrated();
      accel.resetIntegrated();

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
void handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {  
}

void handleNoteOff(uint8_t channel, uint8_t note) {
}

void handleControlChange(uint8_t channel, uint8_t number, uint8_t value) {
  statusLed(1);
  if( number == 2) {magnitudeScaling = (float)value/64; return;} //disable all monitoring
  else if( number >=10 && number<12){ tiltScaling = (float)value/64; return;} //monitor raw data
  else if( number >=20 && number<22){
     accel.x.setRawScale ( (float)value/64 ); 
     accel.y.setRawScale ( (float)value/64 ); 
     accel.z.setRawScale ( (float)value/64 ); 
     return;
     } 
  else if( number >=30 && number<32){ 
    gyro.x.setRawScale ( (float)value/64 ); 
     gyro.y.setRawScale ( (float)value/64 ); 
     gyro.z.setRawScale ( (float)value/64 ); 
    return;
    } 
  else if( number >=40 && number<42){ 
    gyro.x.setIntegratedScale ( (float)value/64 ); 
     gyro.y.setIntegratedScale ( (float)value/64 ); 
     gyro.z.setIntegratedScale ( (float)value/64 ); 
    return;
    } else if( number == 100){ 
     for(int i = 2; i<15; i++){
      cc[i].setMinInterval(value*2);
     }
    return;
    } 
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