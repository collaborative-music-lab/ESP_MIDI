/*
void interpolateColor(int octave);
void setMode(InstrumentMode mode)
void handleOctaveTouchpad(int touchpadIndex, byte state)
void handleMonoOctaveTouchpad(int touchpadIndex, byte state)
void handleMonophonicTouchpad(int touchpadIndex, byte state)
void handlePolyphonicTouchpad(int touchpadIndex, byte state)
void sendStoredParameters()
CCParameter
*/
struct CCParameter {
    const char* name;   // Parameter name (optional, for debugging/logging)
    int ccNumber;       // Associated MIDI CC number
    int* valuePointer;  // Pointer to the value in ParameterObject
    int minValue;       // Minimum value (scaled)
    int maxValue;       // Maximum value (scaled)
};

// Define the mapping
CCParameter ccParameters[] = {
    {"Lower Threshold 0", 96, &parameters.lowerThresholds[0], 0, 2000},
    {"Lower Threshold 1", 97, &parameters.lowerThresholds[1], 0, 2000},
    {"Upper Threshold 0", 107, &parameters.upperThresholds[0], 0, 2000},
    {"Upper Threshold 1", 108, &parameters.upperThresholds[1], 0, 2000},
    {"Mode", 31, reinterpret_cast<int*>(&parameters.mode), 0, 1},
    {"Polyphonic Note 0", 40, reinterpret_cast<int*>(&parameters.polyphonicNotes[0]), 0, 127},
    {"Smoothing Alpha", 118, &parameters.smoothingAlpha, 0, 65536},
    {"Delta Threshold", 119, &parameters.deltaThreshold, 0, 127},
    // Add more parameters as needed
};
const int numCCParameters = sizeof(ccParameters) / sizeof(CCParameter);

void interpolateColor(int octave);

/**********
MONOPHONIC
**********/

// Define the bit array for touchpad states
unsigned int padState = 0;
int octaveShift = 0;

// Map bit patterns to MIDI notes (default to 0 for unused patterns)
byte fingeringToMidiNote[256] = {0};

// Current active MIDI note
byte currentNote = 0;

// Populate the fingering-to-MIDI note mapping
void setupFingeringToMidiNote() {
    fingeringToMidiNote[0b00000000] = 0;    // No fingers pressed (rest)
    fingeringToMidiNote[0b10000000] = 69;  // Left thumb pressed = A4
    fingeringToMidiNote[0b11000000] = 71;  // Left thumb + index = B4
    fingeringToMidiNote[0b11100000] = 72;  // Left thumb + index + middle = C5
    fingeringToMidiNote[0b11110000] = 74;  // Left thumb + index + middle + ring = D5
    fingeringToMidiNote[0b11111000] = 76;  // Left fingers + right index = E5
    fingeringToMidiNote[0b11111100] = 77;  // Left fingers + right index + middle = F5
    fingeringToMidiNote[0b11111110] = 79;  // Left fingers + right hand = G5
    fingeringToMidiNote[0b11111111] = 81;  // All fingers pressed = A5
}

void handleOctaveTouchpad(int touchpadIndex, byte state) {
    if (touchpadIndex == 9 && state == 1) { // A6 (Octave Up)
        octaveShift += 1;
        Serial.printf("Octave shifted up: %d\n", octaveShift);
    } else if (touchpadIndex == 8 && state == 1) { // A5 (Octave Down)
        octaveShift -= 1;
        Serial.printf("Octave shifted down: %d\n", octaveShift);
    }
    if(octaveShift > 3) octaveShift = 4;
    else if(octaveShift<-3) octaveShift = -4;
    interpolateColor(octaveShift);
}

void handleMonoOctaveTouchpad(int touchpadIndex, byte state) {
    if (touchpadIndex == 9 && state == 1) { // A6 (Octave Up)
        octaveShift = 1;
        Serial.printf("Octave shifted up: %d\n", octaveShift);
    } else if (touchpadIndex == 8 && state == 1) { // A5 (Octave Down)
        octaveShift = -1;
        Serial.printf("Octave shifted down: %d\n", octaveShift);
    }
    else octaveShift = 0;
    interpolateColor(octaveShift);
}

void interpolateColor(int octave) {
    // Map octave range -4 to +4 to interpolation ratio (0.0 to 1.0)
    float ratio = (octave + 4) / 8.0;

    // RGB values for indigo and orange
    int indigo[3] = {75, 0, 130};    // RGB for -4
    int orange[3] = {255, 165, 0};  // RGB for +4
    int rgb[3] = {0,0,0};

    // Interpolate each color component
    for (int i = 0; i < 3; i++) {
        rgb[i] = indigo[i] + ratio * (orange[i] - indigo[i]);
    }

    leds[0] = CRGB(rgb[0], rgb[1], rgb[2]);
    FastLED.show();
}


void handleMonophonicTouchpad(int touchpadIndex, byte state) {
  if( touchpadIndex == 8 || touchpadIndex == 9){
    handleMonoOctaveTouchpad(touchpadIndex, state);
    return;
  }

    static const int touchpadBitMapping[8] = {7, 6, 5, 4, 3, 2, 1, 0}; // Left thumb to right pinky
    int bitPosition = touchpadBitMapping[touchpadIndex];

    // Update the bit array
    if (state == 1) padState |= (1 << bitPosition); // Set the bit
    else  padState &= ~(1 << bitPosition); // Clear the bit

    // Look up the base MIDI note for the current fingering
    byte baseNote = fingeringToMidiNote[padState];
    byte newNote = baseNote + octaveShift*12; // Apply the octave shift

    // Handle MIDI Note On/Off events
    if (newNote != currentNote) {
        if (currentNote != 0) {
            sendMidiNoteOff(currentNote); // Stop the previous note
        }
        if (newNote != 0) {
            sendMidiNoteOn(newNote, 127); // Play the new note
        }
        currentNote = newNote; // Update the current note
    }
}

int polyphonicNotes[8] = {60, 62, 64, 65, 67, 69, 71, 72}; // Default scale

void setPolyphonicNote(int index, int note) {
    if (index >= 0 && index < 8) {
        polyphonicNotes[index] = note;
    }
}

void handlePolyphonicTouchpad(int touchpadIndex, byte state) {
  if( touchpadIndex == 8 || touchpadIndex == 9){
    handleOctaveTouchpad(touchpadIndex, state);
    return;
  }
    if (touchpadIndex >= 0 && touchpadIndex < 8) {
        int note = polyphonicNotes[touchpadIndex];
        sendMidiNoteOn(note, 127);
        // Logic for sending Note Off can be added based on touchpad release
    } else if (touchpadIndex == 9) { // A5 (Octave Down)
        sendMidiCC(14, 127); // Example CC for octave down
    } else if (touchpadIndex == 10) { // A6 (Octave Up)
        sendMidiCC(15, 127); // Example CC for octave up
    }
}

void sendStoredParameters() {
    for (int i = 0; i < numCCParameters; i++) {
        // Scale the stored value to MIDI range
        int scaledValue = map(*(ccParameters[i].valuePointer),
                              ccParameters[i].minValue, ccParameters[i].maxValue, 0, 127);

        // Send the CC message
        sendMidiCC(ccParameters[i].ccNumber, scaledValue);
    }
}

void updateParameterValue(uint8_t channel, uint8_t number, uint8_t value) {
    for (int i = 0; i < numCCParameters; i++) {
        if (ccParameters[i].ccNumber == number) {
            // Scale the value to the parameter's range
            int scaledValue = map(value, 0, 127, ccParameters[i].minValue, ccParameters[i].maxValue);

            // Update the parameter value
            *(ccParameters[i].valuePointer) = scaledValue;

            //Serial.printf("Updated %s (CC %d) to %d\n",
            //             ccParameters[i].name, ccParameters[i].ccNumber, scaledValue);

            // Save parameters to Preferences
            preferences.begin("params", false);
            parameters.saveToPreferences(preferences);
            preferences.end();

            return; // Exit after handling the message
        }
    }

    //Serial.printf("Unhandled CC %d\n", number);
}
