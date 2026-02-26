
struct IMidiOut {
  virtual void noteOn(uint8_t ch, uint8_t note, uint8_t vel) = 0;
  virtual void noteOff(uint8_t ch, uint8_t note, uint8_t vel) = 0;
  virtual void cc(uint8_t ch, uint8_t ccNum, uint8_t value) = 0;
  virtual ~IMidiOut() {}
};

// -------------------- UTILS --------------------

static inline float clampf(float x, float a, float b) { return (x < a) ? a : (x > b) ? b : x; }
static inline uint8_t clampu8(int x) { return (x < 0) ? 0 : (x > 255) ? 255 : (uint8_t)x; }

// Simple “soft log” mapping for velocity from a positive metric.
static inline uint8_t metricToVel(float m, float mMin, float mMax) {
  if (m <= mMin) return 1;
  if (m >= mMax) return 127;
  float t = (m - mMin) / (mMax - mMin);          // 0..1
  // log-ish curve without calling logf:
  // y = t / (t + k*(1-t)) with k>1 makes it more compressive near 1
  const float k = 6.0f;
  float y = t / (t + k * (1.0f - t));
  int vel = (int)lroundf(1.0f + 126.0f * y);
  return (uint8_t)clampf(vel, 1, 127);
}

#pragma once
#include <Arduino.h>

class Tine {
public:
  // You provide these.
  using NoteOnFn  = void (*)(uint8_t vel);
  using NoteOffFn = void (*)();
  using CCFn      = void (*)(uint8_t value);

  Tine(uint8_t analogPin, NoteOnFn onFn, NoteOffFn offFn, CCFn ccFn)
  : _pin(analogPin), _noteOn(onFn), _noteOff(offFn), _cc(ccFn) {}

  void begin() {
    _active = false;

    _dc = 0.0f;
    _ac = 0.0f;
    _env = 0.0f;
    _prevAc = 0.0f;

    _recentPeakEnv = 0.0f;
    _belowOffSinceMs = 0;

    _lastCcMs = 0;
    _lastCcValue = 255;
  }

  // Call as fast as you like (1–5 kHz is fine if your ADC is fast enough).
  void update() {
    const uint32_t nowMs = millis();
    const int raw = analogRead(_pin);

    // --- split DC + AC ---
    // DC low-pass
    _dc += DC_ALPHA * ((float)raw - _dc);

    // AC high-pass (raw - DC)
    _ac = (float)raw - _dc;

    // envelope of AC
    const float absAc = fabsf(_ac);
    _env += ENV_ALPHA * (absAc - _env);

    // keep a slowly-decaying recent peak for "sudden damping" detection
    if (_env > _recentPeakEnv) _recentPeakEnv = _env;
    _recentPeakEnv *= PEAK_DECAY;

    // slope metric for fast velocity
    const float slope = _ac - _prevAc;
    _prevAc = _ac;

    // --- DC -> CC (rate limited, deadband) ---
    maybeSendCc(nowMs);

    // --- note state machine ---
    if (!_active) {
      if (_env >= ONSET_ENV_THRESH && fabsf(slope) >= ONSET_SLOPE_THRESH) {
        const uint8_t vel = slopeToVelocity(fabsf(slope));
        if (_noteOn) _noteOn(vel);

        _active = true;
        _belowOffSinceMs = 0;
      }
    } else {
      const bool suddenDrop =
        (_recentPeakEnv > 1.0f) && (_env < DAMP_DROP_FRAC * _recentPeakEnv);

      const bool below = (_env < OFF_ENV_THRESH);
      if (below) {
        if (_belowOffSinceMs == 0) _belowOffSinceMs = nowMs;
      } else {
        _belowOffSinceMs = 0;
      }

      const bool heldLowLongEnough =
        (_belowOffSinceMs != 0) && (nowMs - _belowOffSinceMs >= OFF_HOLD_MS);

      if (suddenDrop || heldLowLongEnough) {
        if (_noteOff) _noteOff();
        _active = false;

        _belowOffSinceMs = 0;
        _recentPeakEnv = 0.0f;
      }
    }
  }

  // Raw-ish signals for your own mapping/debug
  bool  active() const { return _active; }
  float dc()     const { return _dc; }   // ADC units
  float ac()     const { return _ac; }   // ADC units
  float env()    const { return _env; }  // ADC units

  // -------------------- Simple tuning knobs --------------------
  // Filtering
  float DC_ALPHA   = 0.02f;  // smaller = smoother DC (more lag)
  float ENV_ALPHA  = 0.25f;  // envelope smoothing (bigger = faster)

  // Onset / Note-on
  float ONSET_ENV_THRESH   = 25.0f;
  float ONSET_SLOPE_THRESH = 12.0f;

  // Velocity mapping from |slope|
  float VEL_METRIC_MIN = 10.0f;
  float VEL_METRIC_MAX = 220.0f;

  // Note-off
  float OFF_ENV_THRESH = 8.0f;
  uint16_t OFF_HOLD_MS = 140;
  float DAMP_DROP_FRAC = 0.55f; // sudden damping if env < frac * recentPeak
  float PEAK_DECAY     = 0.995f;

  // CC sending (from DC)
  uint16_t CC_INTERVAL_MS = 10;  // max rate (10ms = 100 Hz)
  uint8_t  CC_DEADBAND    = 1;   // steps
  uint16_t ADC_MAX        = 4095;

  // If you want to scale/offset DC before mapping to 0..127
  float DC_SCALE  = 1.0f;
  float DC_OFFSET = 0.0f;

private:
  void maybeSendCc(uint32_t nowMs) {
    if (!_cc) return;
    if (nowMs - _lastCcMs < CC_INTERVAL_MS) return;

    float x = (_dc * DC_SCALE) + DC_OFFSET;

    // default normalization: 0..ADC_MAX -> 0..127
    float t = x / (float)ADC_MAX;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    const uint8_t v = (uint8_t)lroundf(127.0f * t);

    if (_lastCcValue == 255 || (uint8_t)abs((int)v - (int)_lastCcValue) >= CC_DEADBAND) {
      _cc(v);
      _lastCcValue = v;
      _lastCcMs = nowMs;
    }
  }

  uint8_t slopeToVelocity(float m) const {
    if (m <= VEL_METRIC_MIN) return 1;
    if (m >= VEL_METRIC_MAX) return 127;

    float t = (m - VEL_METRIC_MIN) / (VEL_METRIC_MAX - VEL_METRIC_MIN); // 0..1

    // compressive curve without log():
    // y = t / (t + k*(1-t)) ; k>1 compresses near 1
    const float k = 6.0f;
    float y = t / (t + k * (1.0f - t));

    int vel = (int)lroundf(1.0f + 126.0f * y);
    if (vel < 1) vel = 1;
    if (vel > 127) vel = 127;
    return (uint8_t)vel;
  }

  uint8_t _pin;

  NoteOnFn  _noteOn = nullptr;
  NoteOffFn _noteOff = nullptr;
  CCFn      _cc = nullptr;

  bool _active = false;

  float _dc = 0.0f;
  float _ac = 0.0f;
  float _env = 0.0f;
  float _prevAc = 0.0f;

  float _recentPeakEnv = 0.0f;
  uint32_t _belowOffSinceMs = 0;

  uint32_t _lastCcMs = 0;
  uint8_t  _lastCcValue = 255;
};