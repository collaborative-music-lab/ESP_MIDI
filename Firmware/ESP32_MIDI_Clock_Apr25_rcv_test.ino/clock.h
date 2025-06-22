byte clockPin = 33;

// Define a hardware timer pointer
hw_timer_t * timer = NULL;
volatile unsigned long beatCount = 0; // Counter incremented in the ISR
volatile byte timerActive = 0;
// A mutex (critical section) to protect shared variables between ISR and main loop.
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


// This function is the interrupt service routine (ISR)
// It is triggered by the hardware timer.
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  beatCount++;
  timerActive = 2;
  portEXIT_CRITICAL_ISR(&timerMux);
  digitalWrite(clockPin, HIGH);
}

void timerSetup() {
  // Create hardware timer
    timer = timerBegin(1000000);  // Timer 0, divider 8 (10 microsecond tick)
    timerAttachInterrupt(timer, &onTimer);  // Attach callback function
    timerAlarm(timer,500000, true, 0);  // timer, count to call onTimer,
  // Repeat the alarm (third parameter) with unlimited count = 0 (fourth parameter).

  pinMode( clockPin, OUTPUT);
}

void timerLoop(){
  static uint32_t timerStart = 0;

  if(timerActive == 2){
    timerActive = 1;
    timerStart = millis();
  }
  else if(timerActive == 1){
    if(millis()-timerStart > 5){
      digitalWrite( clockPin, LOW);
      MidiSerial.write(0xF8);
      timerActive = 0;
      Serial.println("clock");
    }
  }
}