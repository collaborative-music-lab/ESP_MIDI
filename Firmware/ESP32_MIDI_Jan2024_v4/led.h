#include <stdint.h>
#ifndef LEDS_H
#define LEDS_H

#include <FastLED.h>

// Number of LEDs and pin definition
#define NUM_LEDS 2
#define LED_PIN 13
//21 is built in LED

// LED class abstraction
class WS2812LED {
private:
    int index;           // Index of the LED in the array
    static CRGB leds[];  // Shared FastLED array for all LEDs

public:
    // Constructor
    WS2812LED(int ledIndex) : index(ledIndex) {}

    // Set color using RGB values
    void color(int r, int g, int b) {
        leds[index] = CRGB(r, g, b);
    }

    // Get the current color
    void getColor(int &r, int &g, int &b) const {
        r = leds[index].r;
        g = leds[index].g;
        b = leds[index].b;
    }

    // Static method to initialize FastLED
    static void begin() {
        FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS);
        FastLED.clear();
        FastLED.show();
    }

    static void brightness(uint8_t val){
      FastLED.setBrightness(val);
    }

    // Static method to display updated LED colors
    static void show() {
        FastLED.show();
    }
};

// Define the shared CRGB array
CRGB WS2812LED::leds[NUM_LEDS];

#endif // LEDS_H