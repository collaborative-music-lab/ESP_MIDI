/*
Simple class for managing a single RGB LED on Arduino.

*/

class LED {
  public:
    LED(byte rPin, byte gPin, byte bPin) {
      this->rPin = rPin;
      this->gPin = gPin;
      this->bPin = bPin;
      pinMode(rPin, OUTPUT);
      pinMode(gPin, OUTPUT);
      pinMode(bPin, OUTPUT);
    }

    void set(byte r, byte g, byte b) {
      color[0] = r;
      color[1] = g;
      color[2] = b;
    }

    void setRed(byte r) { color[0] = r; }
    void setGreen(byte g) { color[1] = g; }
    void setBlue(byte b) { color[2] = b; }

    void loop() {
      for (byte i = 0; i < 3; i++) {
        curColor[i] = curColor[i] * smoothing + color[i] * (1 - smoothing);
      }
      analogWrite(rPin, curColor[0]);
      analogWrite(gPin, curColor[1]);
      analogWrite(bPin, curColor[2]);
    }

  private:
    byte rPin, gPin, bPin;
    byte color[3] = {0, 0, 0};
    byte curColor[3] = {0, 0, 0};
    float smoothing = 0.95;
};
