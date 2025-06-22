#include "esp32-hal.h"
class CapSense {
  private:
    int pin;                // Pin number for the sensor
    int baseline;           // Baseline capacitance value
    int value;              // Current sensor reading
    int sensorNumber;       // Sensor ID for printing
    uint32_t baselineTimer = 0;
    

  public:
    int raw;
    bool state;             // Current state (on/off)
    bool change;
    int upperThreshold;     // Threshold for turning ON
    int lowerThreshold;     // Threshold for turning OFF
    bool newNote = false;
    byte velocity=0;

    // Constructor
    CapSense(int sensorPin, int sensorNum, int upperThresh, int lowerThresh)
        : pin(sensorPin), sensorNumber(sensorNum), upperThreshold(upperThresh), lowerThreshold(lowerThresh), state(false) {
        baseline = touchRead(pin); // Initialize the baseline
    }

    // Update the sensor reading and evaluate state
    void update() {
      raw = touchRead(pin); // Read the current value

      if(millis()-baselineTimer > 100){
        baselineTimer = millis();
        if(raw < baseline) baseline = raw;
        else baseline = baseline+1;
      }
      
      int prevValue = value;
      value = raw - baseline;

      if(prevValue-value > 0 && prevValue > upperThreshold && !state){
        velocity = prevValue - upperThreshold;
        newNote = true;
        state= true;
        change = true;
      }
    
      else if (state && value < lowerThreshold) { // Transition to OFF
          state = false;
          change = true;
      }
    }

    // Query the current state (on/off)
    bool getState() const {
        return state;
    }
    bool getChange() {
      if(change){
        change = false;
        return true;
      }
      return false;
    }
    bool getNote(){
      if(newNote) {
        change = false;
        return true;
      }
      else return false;
    }

    // Query the current sensor value
    int getValue() const {
        return value;
    }

    // Query the current sensor value
    int getBaseline() const {
        return baseline;
    }

    // Optionally, update the baseline (if necessary)
    void calibrate() {
        baseline = touchRead(pin); // Update baseline with current reading
    }
};

// Pins for the sensors
const int touchPins[11] = {8, 9, 10, 5, 4, 3, 2, 1, 7, 13, 6};

// Upper and lower thresholds
const int UPPER_THRESHOLD = 300;
const int LOWER_THRESHOLD = 100;

// Array of CapSense instances
CapSense cap[] = {
    CapSense(touchPins[0], 0, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[1], 1, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[2], 2, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[3], 3, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[4], 4, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[5], 5, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[6], 6, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[7], 7, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[8], 8, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[9], 9, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[10], 10, UPPER_THRESHOLD, LOWER_THRESHOLD)
};

void updateCapSenseThreshold(int ccNumber, int value) {
    int thresholdValue = map(value, 0, 127, 0, 2000); // Map 0–127 to 0–2000
    int sensorIndex;
    bool isUpperThreshold;

    if (ccNumber >= 96 && ccNumber <= 106) {
        // Lower thresholds for touchpads 0–10
        sensorIndex = ccNumber - 96;
        isUpperThreshold = false;
    } else if (ccNumber >= 107 && ccNumber <= 117) {
        // Upper thresholds for touchpads 0–10
        sensorIndex = ccNumber - 107;
        isUpperThreshold = true;
    } else {
        // Out of range, ignore
        return;
    }

    // Update the threshold for the correct sensor
    if (isUpperThreshold) {
        cap[sensorIndex].upperThreshold = thresholdValue;
        Serial.printf("Updated upperThreshold for touchpad %d to %d\n", sensorIndex, thresholdValue);
    } else {
        cap[sensorIndex].lowerThreshold = thresholdValue;
        Serial.printf("Updated lowerThreshold for touchpad %d to %d\n", sensorIndex, thresholdValue);
    }
}