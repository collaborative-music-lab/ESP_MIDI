/*
class ParameterObject
*/

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <Preferences.h>

struct ParameterObject {
    // Thresholds for touchpads
    int lowerThresholds[11];
    int upperThresholds[11];

    // Mode (0 = Monophonic, 1 = Polyphonic)
    byte mode;

    // Polyphonic note mappings (8 touchpads)
    byte polyphonicNotes[8];

    // Other parameters
    int smoothingAlpha;   // Alpha for low-pass filter (scaled as int, e.g., 0.2 = 200)
    int deltaThreshold;   // Minimum delta for sending CC updates

    // Load parameters from Preferences
    void loadFromPreferences(Preferences& preferences) {
        if (preferences.isKey("lowerThresholds")) {
            preferences.getBytes("lowerThresholds", lowerThresholds, sizeof(lowerThresholds));
            preferences.getBytes("upperThresholds", upperThresholds, sizeof(upperThresholds));
            mode = preferences.getUChar("mode", 0); // Default to Monophonic
            preferences.getBytes("polyphonicNotes", polyphonicNotes, sizeof(polyphonicNotes));
            smoothingAlpha = preferences.getUInt("smoothingAlpha", 200); // Default alpha = 200
            deltaThreshold = preferences.getUInt("deltaThreshold", 2);   // Default delta = 2
            //Serial.println("Parameters loaded from Preferences.");
        } else {
            //Serial.println("No stored parameters found. Using defaults.");
            setDefaults(); // Use defaults if no data exists
        }
    }

    // Save parameters to Preferences
    void saveToPreferences(Preferences& preferences) {
        preferences.putBytes("lowerThresholds", lowerThresholds, sizeof(lowerThresholds));
        preferences.putBytes("upperThresholds", upperThresholds, sizeof(upperThresholds));
        preferences.putUChar("mode", mode);
        preferences.putBytes("polyphonicNotes", polyphonicNotes, sizeof(polyphonicNotes));
        preferences.putUInt("smoothingAlpha", smoothingAlpha);
        preferences.putUInt("deltaThreshold", deltaThreshold);
        //Serial.println("Parameters saved to Preferences.");
    }

    // Set default values
    void setDefaults() {
        for (int i = 0; i < 11; i++) {
            lowerThresholds[i] = 100;
            upperThresholds[i] = 1000;
        }
        mode = 0; // Default to Monophonic
        for (int i = 0; i < 8; i++) {
            polyphonicNotes[i] = 60 + i; // Default scale (C4, D4, E4, ...)
        }
        smoothingAlpha = 200; // Default alpha = 200
        deltaThreshold = 2;   // Default delta = 2
    }
};

#endif