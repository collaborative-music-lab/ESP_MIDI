#pragma once
#include <Arduino.h>

class TineRingBuffer {
public:
  // capacity must be > 0
  TineRingBuffer(uint8_t adcPin, int16_t* storage, uint16_t capacity)
  : _pin(adcPin), _buf(storage), _cap(capacity) {}

  void begin() {
    _w = 0;
    _count = 0;
  }

  uint8_t pin() const { return _pin; }

  // push one sample (overwrites oldest when full)
  void push(int16_t x) {
    _buf[_w] = x;
    _w = (_w + 1) % _cap;

    if (_count < _cap) _count++;
  }

  // how many samples currently available (<= capacity)
  uint16_t available() const { return _count; }

  // read the i-th newest sample:
  // i=0 -> newest, i=available-1 -> oldest
  int16_t newest(uint16_t i = 0) const {
    if (_count == 0) return 0;
    if (i >= _count) i = _count - 1;

    // newest is at write-1
    int32_t idx = (int32_t)_w - 1 - (int32_t)i;
    while (idx < 0) idx += _cap;
    return _buf[idx];
  }

  // copy up to n samples into dst in chronological order (oldest -> newest)
  // returns number copied
  uint16_t copyChrono(int16_t* dst, uint16_t n) const {
    const uint16_t k = (_count < n) ? _count : n;
    if (k == 0) return 0;

    const uint16_t start = (_w + _cap - _count) % _cap; // oldest index
    for (uint16_t i = 0; i < k; i++) {
      dst[i] = _buf[(start + i) % _cap];
    }
    return k;
  }

private:
  uint8_t _pin;
  int16_t* _buf;
  uint16_t _cap;

  uint16_t _w = 0;       // next write index
  uint16_t _count = 0;   // number of valid samples
};