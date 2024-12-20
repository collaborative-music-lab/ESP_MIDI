class CapSense {
private:
    int pin;                // Pin number for the sensor
    int baseline;           // Baseline capacitance value
    int value;              // Current sensor reading
    bool state;             // Current state (on/off)
    int upperThreshold;     // Threshold for turning ON
    int lowerThreshold;     // Threshold for turning OFF
    int sensorNumber;       // Sensor ID for printing

public:
    // Constructor
    CapSense(int sensorPin, int sensorNum, int upperThresh, int lowerThresh)
        : pin(sensorPin), sensorNumber(sensorNum), upperThreshold(upperThresh), lowerThreshold(lowerThresh), state(false) {
        baseline = touchRead(pin); // Initialize the baseline
    }

    // Update the sensor reading and evaluate state
    void update() {
        value = touchRead(pin); // Read the current value
        if(value < baseline) baseline = value;
        else baseline = baseline+1;
        value = value - baseline;
      

        if (!state && value > upperThreshold) { // Transition to ON
            state = true;
            if( SERIAL_DEBUG ){
            Serial.print("Sensor ");
            Serial.print(sensorNumber);
            Serial.println(" ON");
            }

            if(ENABLE_USB_MIDI){
              sendMidiNoteOn( sensorNumber, 127 );
            }
        } else if (state && value < lowerThreshold) { // Transition to OFF
            state = false;
            if( SERIAL_DEBUG ){
            Serial.print("Sensor ");
            Serial.print(sensorNumber);
            Serial.println(" OFF");
            }
            if(ENABLE_USB_MIDI){
              sendMidiNoteOff( sensorNumber );
            }
        }
    }

    // Query the current state (on/off)
    bool getState() const {
        return state;
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