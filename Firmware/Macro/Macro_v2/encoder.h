// based on https://github.com/igorantolic/ai-esp32-rotary-encoder
// which was itself based on https://github.com/marcmerlin/IoTuz code - extracted and modified Encoder code
// 
// 

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

// based on https://github.com/igorantolic/ai-esp32-rotary-encoder
// which was itself based on https://github.com/marcmerlin/IoTuz code - extracted and modified Encoder code
// 
// 


void IRAM_ATTR Esp32Encoder::readEncoder_ISR()
{	
	portENTER_CRITICAL_ISR(&(this->mux));
  this->ISRflag = 1;
	this->old_AB <<= 2;                   //remember previous state
  this->old_AB;
	int8_t ENC_PORT = (
	  (digitalRead(this->encoderBPin)) ? (1 << 1) : 0) + 
	  ((digitalRead(this->encoderAPin)) ? (1 << 0) : 0
	  );
	this->old_AB |= ( ENC_PORT & 0x03 );  //add current state
	this->encoder0Pos += ( this->enc_states[( this->old_AB & 0x0f )]);		
	portEXIT_CRITICAL_ISR(&(this->mux));
}//readEncoder_ISR

Esp32Encoder::Esp32Encoder(uint8_t _APin, uint8_t _BPin, uint8_t _ButtonPin, uint8_t _divider, uint8_t _num)
{
	this->old_AB = 0;
	
	this->encoderAPin = _APin;
	this->encoderBPin = _BPin;
	this->encoderButtonPin = _ButtonPin;
  this->divider = _divider;
  this->ccNumber = _num;
	pinMode(this->encoderAPin, INPUT_PULLUP);
	pinMode(this->encoderBPin, INPUT_PULLUP);

}//Esp32Encoder

//Esp32Encoder::Esp32Encoder(uint8_t _APin, uint8_t _BPin, uint8_t _ButtonPin, uint8_t _divider)
//{
//  this->old_AB = 0;
//  
//  this->encoderAPin = _APin;
//  this->encoderBPin = _BPin;
//  this->encoderButtonPin = _ButtonPin;
//  this->divider =_divider;
//  pinMode(this->encoderAPin, INPUT);
//  pinMode(this->encoderBPin, INPUT);
//
//}//Esp32Encoder


int16_t Esp32Encoder::readEncoder() {
  int32_t out = 0;  // default when not enough ticks yet
  portENTER_CRITICAL(&mux);

  if (divider <= 0) divider = 1;  // safety
  int32_t pos = encoder0Pos;

  // Only report when we've crossed a full 'divider' step (in either direction)
  if (pos >= divider || pos <= -divider) {
    out = pos / divider;                            // number of steps to report
    encoder0Pos = pos - out * divider;              // keep leftover ticks
    ISRflag = 0;                                    // if you actually use this
  }

  portEXIT_CRITICAL(&mux);
  return static_cast<int16_t>(out);
}

int16_t Esp32Encoder::delta() {   return (readEncoder()); }

int32_t Esp32Encoder::count()
{
  int encoderDelta = readEncoder();
  if (encoderDelta == 0) return this->_count ;
  int prevRaw = this->_raw;
  this->_raw+=encoderDelta;
  if(  abs(this->_raw) > this->divider-1 || (abs(this->_raw)-abs(prevRaw)<0)){
    this->_raw=0;
    encoderDelta > 0 ? this->_count++ : this->_count--;
  }
  return (this->_count);
}

int16_t Esp32Encoder::encoderChanged() 
{
	int16_t _encoder0Pos = readEncoder();
	
	int16_t encoder0Diff = _encoder0Pos - this->lastReadEncoder0Pos;

	this->lastReadEncoder0Pos = _encoder0Pos;
	return encoder0Diff;
}//readEncoder

void Esp32Encoder::begin(void (*ISR_callback)(void))
{
	this->previous_butt_state = 0;
	pinMode(this->encoderButtonPin, INPUT_PULLUP);
  pinMode(this->encoderAPin, INPUT_PULLUP);
  pinMode(this->encoderBPin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderAPin), ISR_callback, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderBPin), ISR_callback, CHANGE);

  Serial.println(encoderAPin);
  Serial.println(encoderBPin);
}//begin

byte Esp32Encoder::available(){
  return ISRflag;
}

ButtonState Esp32Encoder::button()
{
	uint8_t butt_state = 255;
  byte curReading = digitalRead(this->encoderButtonPin);
  this->button_history = (this->button_history<<1) + curReading;
  if(this->button_history == this->onPattern ) butt_state = 1;
  else if (this->button_history == this->offPattern ) butt_state = 0;

  if( butt_state  <=1  ){
    previous_butt_state = butt_state;
     switch(butt_state){
      case 1: return  BUT_PUSHED;
      case 0: return BUT_RELEASED;
     }
  }else {
    switch(previous_butt_state){
      case 0: return  BUT_DOWN;
      case 1: return BUT_UP;
     }
  }
}//button


