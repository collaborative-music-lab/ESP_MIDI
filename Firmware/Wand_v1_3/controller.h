/*
CC class for sending MIDI CC messages
- call CController.send(value) to send a message
- inLow and inHigh values set a range which will be mapped to 0-127
- large changes in CC value will be sent immmediately
- otherwise, a lowpass filter with alphaFixed will smooth the CC value
- call updateAlpha(float 0-1) to change the alpha.
*/

class CController {
  private:
    int ccNumber;
    int lastCCValue = -1;
    unsigned long lastSendTime = 0;
    int minInterval;
    int inHigh;
    int inLow;

    float smoothedValue = 0.0;
    float smoothedDerivative = 0.0;

    // One Euro parameters
    float minCutoff;
    float beta;
    float dCutoff;
    float lastRawValue = 0.0;
    float lastFilteredValue = 0.0;
    unsigned long lastTime = 0;

    int deltaThreshold;

    float alpha(float cutoff, float dt) {
      float tau = 1.0 / (2 * PI * cutoff);
      return 1.0 / (1.0 + tau / dt);
    }

    float lowpass(float current, float previous, float alphaVal) {
      return alphaVal * current + (1.0 - alphaVal) * previous;
    }

  public:
    CController(int ccNumber, int minInterval = 20, int inLow = 0, int inHigh = 127, int deltaThreshold = 20,
                float minCutoff = 1.0, float beta = 0.02, float dCutoff = 1.0)
      : ccNumber(ccNumber), lastCCValue(-1), lastSendTime(0), minInterval(minInterval),
        inLow(inLow), inHigh(inHigh), deltaThreshold(deltaThreshold),
        minCutoff(minCutoff), beta(beta), dCutoff(dCutoff), lastTime(0) {}

    byte debug = 0;

    void send(int val) {
      val = constrain(val, inLow, inHigh);
      float input = map(val, inLow, inHigh, 0, 511);

      unsigned long now = millis();
      float dt = (lastTime > 0) ? (now - lastTime) / 1000.0 : 0.01; // in seconds
      lastTime = now;

      float dx = (input - lastRawValue) / dt;
      lastRawValue = input;

      float alphaDeriv = alpha(dCutoff, dt);
      smoothedDerivative = lowpass(dx, smoothedDerivative, alphaDeriv);

      float cutoff = minCutoff + beta * fabs(smoothedDerivative);
      float alphaSignal = alpha(cutoff, dt);
      smoothedValue = lowpass(input, smoothedValue, alphaSignal);

      int filteredInt = (int)(smoothedValue);
      if (abs(filteredInt - lastCCValue) > 4 && (millis() - lastSendTime) >= minInterval) {
        if (ENABLE_USB_MIDI) sendMidiCC(ccNumber, filteredInt / 4);
        else Serial.printf("OneEuro CC: %d, Value: %d\n", ccNumber, filteredInt / 4);
        //else Serial.println(filteredInt / 4);

        lastCCValue = filteredInt;
        lastSendTime = millis();
      }
    }
};