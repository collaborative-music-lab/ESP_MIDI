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
      int lastCCValue;       // Last sent CC value
      unsigned long lastSendTime; // Timestamp of the last CC message sent
      int minInterval;       // Minimum interval between CC messages (ms)
      int inHigh;
      int inLow;

      int32_t smoothedValue; // Smoothed CC value in fixed-point (16.16)
      int deltaThreshold;    // Threshold for value change to send CC
      int alphaFixed;        // Smoothing factor in fixed-point (16.16)


  public:
      // Constructor
      CController(int ccNumber, int minInterval = 20, int inLow = 0, int inHigh = 127, int deltaThreshold = 2, float alpha = 0.2)
          : ccNumber(ccNumber), lastCCValue(-1), lastSendTime(0), minInterval(minInterval),
          inLow(inLow), inHigh(inHigh), smoothedValue(0), deltaThreshold(deltaThreshold)  {
            alphaFixed = alpha * 65536; // Convert alpha to 16.16 fixed-point
          }

      byte debug = 0;

      // Initialize the touchpad (if necessary)
      void init() {
          // Add any initialization logic for the pin here
      }

      // Handle CC value sending
      void send(int val) {
          int ccValue = map(val, inLow, inHigh, 0, 127);
          ccValue = constrain(ccValue, 0, 127);

          //if the value hasn't change, don't send a message
          if( ccValue == lastCCValue ) return;

          // if the new value is significantly different, send a message immediately
          int delta = abs(ccValue - lastCCValue);
          if( delta > deltaThreshold ){
            sendMidiCC(ccNumber, ccValue); // Send the MIDI CC message
            lastCCValue = ccValue;        // Update the last sent value
            lastSendTime = millis();            // Update the last send time
            return;
          }

          //else apply a lowpass filter and look for a change in value
          int32_t mappedValueFixed = ccValue << 16; // Convert to fixed-point (16.16)

          // Apply low-pass filter
          int filteredValue = lowPassFilter(mappedValueFixed) >> 16; // Convert back to integer

          // Check if the change is significant enough or respects rate limit
        if (filteredValue != lastCCValue && (millis() - lastSendTime) >= minInterval) {
            sendMidiCC(ccNumber, filteredValue); // Send the MIDI CC message
            lastCCValue = filteredValue;        // Update the last sent value
            lastSendTime = millis();            // Update the last send time
        }
      }

       // Efficient low-pass filter using fixed-point
    int32_t lowPassFilter(int32_t currentValue) {
        smoothedValue = ((alphaFixed * currentValue) + ((65536 - alphaFixed) * smoothedValue)) >> 16;
        return smoothedValue;
    }

    void updateAlpha(float alpha){
      alphaFixed = alpha * 65536; // Convert alpha to 16.16 fixed-point
    }

};
