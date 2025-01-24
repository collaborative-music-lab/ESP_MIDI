/*
Simple class for managing potentiometers on Arduino. 

Includes:
- auto-scaling to MIDI CC ranges (0-127)
- option to reverse the output if potentiometers are wired backwards
- logic for only sending data when the potentiometer value has changed
  - this uses a changeThreshold which must be exceeded before the value is considered to have changed
  - if the value has not changed, read returns -1 to indicate no change
*/

class Potentiometer {
  public:
    Potentiometer(byte pin, bool reversed=0) {
      this->pin = pin;
      this-> isReversed = reversed;
      pinMode(pin, INPUT);
    }

    int read() { 
      int output = -1;
      val = analogRead(pin) * (1-smoothing) + val * smoothing;
      
      if(abs(val-prevVal) > changeThreshold){
        prevVal = val;
        output = val;
        if( output < minVal ) output = minVal;
        else if (output > maxVal ) output = maxVal;
        output = map(output, minVal, maxVal, 0, 127);
        if( isReversed) output = 127-output;
      }

      return output; 
      }

    void setMinMax(int minVal, int maxVal) { 
      this->minVal = minVal;
      this->maxVal = maxVal;
      }

  private:
    byte pin;
    int changeThreshold = 20;
    int minVal = 0;
    int maxVal = 4000;
    int prevVal;
    int val;
    float smoothing = 0.98;
    bool isReversed;
};
