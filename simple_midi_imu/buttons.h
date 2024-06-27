class Button {
  public:
    Button(byte pin) {
      this->pin = pin;
      pinMode(pin, INPUT_PULLUP); // Assuming a pull-up resistor configuration
      lastDebounceTime = 0;
      debounceDelay = 20; // 50 ms debounce delay
      lastButtonState = HIGH;
    }

    void loop() {
      if ((millis() - lastDebounceTime) > debounceDelay) {
        int reading = digitalRead(pin);
        if (reading != lastButtonState){
          lastDebounceTime = millis();
          lastButtonState = reading;
          if( reading == HIGH) state = PRESSED;
          else state = RELEASED;
        } 
      } 
    }//update

    bool isPressed() {  
      if(state == PRESSED){
        state = DOWN;
        return 1; 
        }
      else return 0;
    }
    bool isReleased() {  
      if(state == RELEASED){
        state = UP;
        return 1; 
        }
      else return 0;
    }
    bool isDown() {  return state == DOWN; }
    bool isUp() { return state == UP; }

  private:
    byte pin;
    int lastButtonState;
    unsigned long lastDebounceTime;
    unsigned long debounceDelay;

    enum ButtonState {
      PRESSED,
      RELEASED,
      DOWN,
      UP
    };

    ButtonState state;
};
