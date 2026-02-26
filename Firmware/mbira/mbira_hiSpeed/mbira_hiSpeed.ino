#include <Arduino.h>
#include "TineRingBuffer.h"

// Your ADC1 pins on ESP32-S3 (GPIO 1..10 are ADC1; 3..7 are fine)
static const uint8_t hallPins[] = {3,4,5,6,7};
static const uint8_t NUM_PINS = 5;
static const uint8_t N = sizeof(hallPins) / sizeof(hallPins[0]);

// Ring buffers: pick a size you like. 256 @ 20kHz total / 5ch ~ 4kHz/ch => ~64ms per tine.
static const uint16_t BUF_LEN = 256;

// Backing storage
static int16_t buf0[BUF_LEN];
static int16_t buf1[BUF_LEN];
static int16_t buf2[BUF_LEN];
static int16_t buf3[BUF_LEN];
static int16_t buf4[BUF_LEN];

// One buffer object per tine
static TineRingBuffer tines[N] = {
  TineRingBuffer(hallPins[0], buf0, BUF_LEN),
  TineRingBuffer(hallPins[1], buf1, BUF_LEN),
  TineRingBuffer(hallPins[2], buf2, BUF_LEN),
  TineRingBuffer(hallPins[3], buf3, BUF_LEN),
  TineRingBuffer(hallPins[4], buf4, BUF_LEN),
};

volatile bool adcDone = false;
adc_continuous_data_t* result;   // ← NO "= nullptr"

void ARDUINO_ISR_ATTR adcComplete() { adcDone = true; }

void setup() {
  Serial.begin(115200);

  for (uint8_t i = 0; i < N; i++) tines[i].begin();

  analogContinuousSetWidth(12);
  analogContinuousSetAtten(ADC_6db);

  // conversions per pin per cycle, sampling frequency (shared), callback
  const uint8_t  conversionsPerPin = 1;
  const uint32_t sampleFreqHz = 20000;

  if (!analogContinuous(hallPins, N, conversionsPerPin, sampleFreqHz, &adcComplete)) {
    Serial.println("analogContinuous failed");
    while (true) delay(1000);
  }
  if (!analogContinuousStart()) {
    Serial.println("analogContinuousStart failed");
    while (true) delay(1000);
  }
}

static inline void routeSampleToTine(uint8_t pin, int raw) {
  // raw is typically 0..4095; store as int16_t
  const int16_t x = (int16_t)raw;

  // simple linear search is fine for 5 pins
  for (uint8_t i = 0; i < N; i++) {
    if (tines[i].pin() == pin) {
      tines[i].push(x);
      return;
    }
  }
}

void loop() {
  if (!adcDone) return;
  adcDone = false;

  if (!analogContinuousRead(&result, 0)) return;

  // result[] has one entry per configured pin (Arduino API: averaged sample fields)  [oai_citation:2‡Espressif Systems](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html?utm_source=chatgpt.com)
  for (int i = 0; i < NUM_PINS; i++) {
    uint8_t pin = result[i].pin;
    int raw     = result[i].avg_read_raw;
    // route sample...
  }

  // Example: read newest sample for tine0
  // int16_t s = tines[0].newest();
}