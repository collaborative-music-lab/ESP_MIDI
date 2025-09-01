// based on https://github.com/igorantolic/ai-esp32-rotary-encoder
// which was itself based on https://github.com/marcmerlin/IoTuz code - extracted and modified Encoder code
// 
// 

#ifndef _Esp32Encoder_h
#define _Esp32Encoder_h

#include "Arduino.h"

// Rotary Encocer
#define DEFAULT_A_PIN 25
#define DEFAULT_B_PIN 26
#define DEFAULT_BUT_PIN 15
#define DEFAULT_DIVIDER 1

typedef enum {
	BUT_DOWN = 0,
	BUT_PUSHED = 1,
	BUT_UP = 2,
	BUT_RELEASED = 3,
} ButtonState;

class Esp32Encoder {
	
private:
	portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
	volatile int16_t encoder0Pos = 0;
	bool _circleValues = false;
	bool isEnabled = true;
  uint32_t _count;
  long _raw;

	uint8_t encoderAPin      = DEFAULT_A_PIN;
	uint8_t encoderBPin      = DEFAULT_B_PIN;
	uint8_t encoderButtonPin = DEFAULT_BUT_PIN;

	volatile uint8_t old_AB;
	int16_t lastReadEncoder0Pos;
  uint8_t divider;
	bool previous_butt_state;
	uint16_t button_history = 0;
	uint16_t onPattern = 1<<15;
	uint16_t offPattern = 1;

	int8_t enc_states[16] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
	void(*ISR_callback)();
	volatile byte ISRflag = 0;

public: 
	Esp32Encoder(
		uint8_t encoderAPin = DEFAULT_A_PIN,
		uint8_t encoderBPin = DEFAULT_B_PIN,
		uint8_t encoderButtonPin = DEFAULT_BUT_PIN,
    uint8_t divider = DEFAULT_DIVIDER,
    uint8_t _num = 50
	);
	void IRAM_ATTR readEncoder_ISR();
	
	void begin(void (*ISR_callback)(void));
	byte available();
	int16_t readEncoder();
  int16_t delta();
  int32_t count();
	int16_t encoderChanged();
  byte ccNumber = 0;
	ButtonState button();

};
#endif

