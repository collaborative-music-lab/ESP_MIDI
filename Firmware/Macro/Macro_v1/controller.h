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
      int ccNumber;          // MIDI CC number associated with this touchpad
      int lastCCValue = 0;       // Last sent CC value
      unsigned long lastSendTime = 0; // Timestamp of the last CC message sent
      int minInterval;       // Minimum interval between CC messages (ms)
      int inHigh;
      int inLow;

      float smoothedValue = 0;   // Smoothed CC value (floating point)
      int deltaThreshold;    // Threshold for value change to send CC

  public:
      // Constructor
      CController(int ccNumber, int minInterval = 20, int inLow = 0, int inHigh = 127, int deltaThreshold = 20, float alpha = 0.1)
          : ccNumber(ccNumber), lastCCValue(-1), lastSendTime(0), minInterval(minInterval),
          inLow(inLow), inHigh(inHigh), smoothedValue(0.0), deltaThreshold(deltaThreshold), alpha(alpha) {}

      byte debug = 0;
      float alpha;           // Smoothing factor (floating point)

      // Initialize the touchpad (if necessary)
      void init() {
          // Add any initialization logic for the pin here
      }

      // Handle CC value sending
      void send(int val) {
          val = constrain(val,inLow, inHigh);
          int ccValue = map(val, inLow, inHigh, 0, 511);
          ccValue = constrain(ccValue, 0, 511);

          // If the value hasn't changed, don't send a message
          // if (abs(ccValue - lastCCValue) <8) return;

          // If the new value is significantly different, send a message immediately
          int delta = 0;//abs(ccValue - lastCCValue);
          if (delta > deltaThreshold) {
              if (ENABLE_USB_MIDI) sendMidiCC(ccNumber, ccValue/4);
              else Serial.printf("CC: %d, %d, and %d\n", ccNumber, ccValue/4, delta);

              lastCCValue = ccValue;
              lastSendTime = millis();
              lowPassFilter(ccValue);
              return;
          }

          // Apply low-pass filter
          float filteredValue = lowPassFilter(ccValue);

          // Check if the change is significant enough or respects rate limit
          if (abs((int)filteredValue - lastCCValue) >4 && (millis() - lastSendTime) >= minInterval) {
              if (ENABLE_USB_MIDI) sendMidiCC(ccNumber, (int)filteredValue/4);
              else Serial.printf("Filtered Send: CC: %d, Value: %d\n", ccNumber, (int)filteredValue/4);

              lastCCValue = (int)filteredValue;
              lastSendTime = millis();
          }
      }

      // Floating-point low-pass filter
      float lowPassFilter(float currentValue) {
          smoothedValue = alpha * currentValue + (1.0 - alpha) * smoothedValue;
          return smoothedValue;
      }

      void updateAlpha(float newAlpha) {
          alpha = newAlpha;
      }
};