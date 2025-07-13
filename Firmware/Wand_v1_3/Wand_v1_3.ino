/*
Wand v1_3
*/

const byte ENABLE_USB_MIDI = 0;
const byte SERIAL_DEBUG = 1;

#include <Wire.h>
#include "USB_MIDI.h"
#include "buttons.h"
#include "controller.h"
#include "LSM6.h"

#include <FastLED.h>
#define NUM_LEDS 10
#define LED_PIN 4
CRGB leds[NUM_LEDS];
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

struct TiltAngles {
  float roll;   // degrees
  float pitch;  // degrees
  float yaw;    // degrees (approximate from tilt)
};

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
  FastLED.clear();
  FastLED.show();
  FastLED.setBrightness(255);  // Max brightness
}

void loop()
{
  readButtons();

  static uint32_t timer = 0;
  int interval = 2; 
  static byte prevValue[] = {0,0,0,0,0};

  if(millis()-timer > interval){
    timer= millis();

    imu.read(); 
    float magnitude = calcMagnitude();

    calcDrumStrike(magnitude);
    return;
    updateLeakyGradient(magnitude, getBaseColorFromButtons());


    cc[0].send(analogRead(1));
    cc[1].send(analogRead(2));
    if(buttonIsPressed == 1){
      cc[2].send(int(magnitude * 100));
    }

    TiltAngles angles = computeTiltAngles(imu.a.x, imu.a.y, imu.a.z);

  cc[3].send(angles.roll);
  cc[4].send(angles.pitch);
  cc[5].send(angles.yaw); 
    
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
    uint8_t scale = map(i, 0, NUM_LEDS - 1, 0, motionLevel);

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

  // Roll: rotation around X axis (-90 to 90)
  angles.roll = atan2(accelY, accelZ) * 180.0 / PI;

  // Pitch: rotation around Y axis (-90 to 90)
  angles.pitch = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0 / PI;

  // Approximate yaw: heading in horizontal plane (not reliable from accel only)
  angles.yaw = atan2(accelY, accelX) * 180.0 / PI;

  return angles;
}

void calcDrumStrike(float magnitude) {
  static float prevMagnitude = 0;
  static bool triggered = false;
  static bool noteSent = false;
  static float peakMagnitude = 0;

  const float triggerThreshold = 0.5;   // strike starts
  const float resetThreshold = 0.1;     // ready to re-arm
  const float peakFalloffThreshold = -0.5; // used to detect end of upward swing

  float delta = magnitude - prevMagnitude;
  prevMagnitude = magnitude;

  static unsigned long lastTriggerTime = 0;
  unsigned long now = millis();

  if (!triggered && delta > triggerThreshold) {
    // Detected rising edge — begin strike
    triggered = true;
    noteSent = false;
    peakMagnitude = magnitude;
    lastTriggerTime = now;
  }

  if (triggered) {
    if (magnitude > peakMagnitude) {
      peakMagnitude = magnitude; // update peak
    }

    // Once the value starts falling quickly, assume we passed the peak
    if (!noteSent && delta < peakFalloffThreshold) {
      uint8_t velocity = map(peakMagnitude * 100, 100, 300, 20, 127);
      velocity = constrain(velocity, 1, 127);
      sendButtonPress(36, velocity);  // Note On
      noteSent = true;
    }

    // Wait until fully settled before resetting
    if (magnitude < resetThreshold) {
      triggered = false;
      noteSent = false;
      sendButtonPress(36, 0);  // Note Off
    }

    // Optional safety timeout
    if (now - lastTriggerTime > 200) {
      triggered = false;
      noteSent = false;
      sendButtonPress(36, 0);  // Note Off (failsafe)
    }
  }
}