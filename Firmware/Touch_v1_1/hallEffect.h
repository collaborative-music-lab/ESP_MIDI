class HallEffectSensor {
  public:
    HallEffectSensor()
      : minVal(1800), maxVal(2200), neutralVal(2000), prevVal(0),
        stableStart(0), lastVal(0) {}

    int update(int analogVal) {
        // 1. Track min and max
        if(analogVal > 5000) return lastVal;
        if (analogVal < minVal) minVal = analogVal;
        if (analogVal > maxVal) maxVal = analogVal;

        // 2. Track neutral only during stable input
        int delta = abs(analogVal - prevVal);
        prevVal = analogVal;

        if(delta > 300 && _triggered == 0) _triggered = 1 ;

        if (delta < stabilityThreshold) {
            if (stableStart == 0) stableStart = millis();

            if (millis() - stableStart > stabilityTimeMs) {
                neutralVal = neutralAlpha * analogVal + (1.0 - neutralAlpha) * neutralVal;
                neutralVal = constrain(neutralVal, minNeutral, maxNeutral);
            }
        } else {
            stableStart = 0;
        }

        // 3. Determine polarity
        bool polarityNegative = abs(minVal - neutralVal) > abs(maxVal - neutralVal);

        int mapped = 0;
        if (polarityNegative && analogVal < neutralVal) {
            mapped = map(analogVal, neutralVal, minVal, 0, 4095);
        } else if (!polarityNegative && analogVal > neutralVal) {
            mapped = map(analogVal, neutralVal, maxVal, 0, 4095);
        }

        lastVal = constrain(mapped, 0, 4095);
        if(_triggered == 2 && lastVal < 200) _triggered = 0;
        return lastVal;
    }

    int triggered(){
      if(_triggered == 1){
        _triggered = 2;
        return 1;
      } 
      else return 0;
    }

    int getLastValue() const {
        return lastVal;
    }

    int getNeutral() const {
        return neutralVal;
    }

  private:
    // Sensor state
    int minVal;
    int maxVal;
    float neutralVal;
    int prevVal;
    unsigned long stableStart;
    int lastVal;
    byte _triggered = 0;

    // Tunable parameters
    const int stabilityThreshold = 4;
    const unsigned long stabilityTimeMs = 300;
    const float neutralAlpha = 0.02;

    const int minNeutral = 1800;
    const int maxNeutral = 2100;
};