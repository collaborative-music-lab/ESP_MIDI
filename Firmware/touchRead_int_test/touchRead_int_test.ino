/*
This is an example how to use Touch Intrrerupts
The bigger the threshold, the more sensible is the touch
*/

int threshold = 100;
bool touchDetected[10];
byte pins[] = {7, 8,9,4, 3,2,1,0, 5,6};
uint32_t baseline[10];

void gotTouch0() { touchDetected[0] = true; }
void gotTouch1() { touchDetected[1] = true; }
void gotTouch2() { touchDetected[2] = true; }
void gotTouch3() { touchDetected[3] = true; }
void gotTouch4() { touchDetected[4] = true; }
void gotTouch5() { touchDetected[5] = true; }
void gotTouch6() { touchDetected[6] = true; }
void gotTouch7() { touchDetected[7] = true; }
void gotTouch8() { touchDetected[8] = true; }
void gotTouch9() { touchDetected[9] = true; }

void setup() {
  Serial.begin(115200);
  delay(1000);  // give me time to bring up serial monitor
  Serial.println("ESP32 Touch Interrupt Test");
  touchAttachInterrupt(1, gotTouch0, threshold);
  touchAttachInterrupt(2, gotTouch1, threshold);
  touchAttachInterrupt(3, gotTouch2, threshold);
  touchAttachInterrupt(4, gotTouch3, threshold);
  touchAttachInterrupt(5, gotTouch4, threshold);
  touchAttachInterrupt(6, gotTouch5, threshold);
  touchAttachInterrupt(7, gotTouch6, threshold);
  touchAttachInterrupt(8, gotTouch7, threshold);
  touchAttachInterrupt(9, gotTouch8, threshold);
  touchAttachInterrupt(10, gotTouch9, threshold);

  for(byte i=0;i<10;i++) baseline[pins[i]] = touchRead(pins[i]);
}

void loop() {
  static uint32_t timer = 0;
  int interval = 10;
  static int count = 0;
  if(millis()-timer > interval){
  for(int i=0;i<8;i++){
    if(touchDetected[pins[i]]){
      int val = touchRead(pins[i]);
      //touchDetected[pins[i]] = false;
      Serial.print(val);
      Serial.print('\t');
      if(val < baseline[pins[i]] + threshold) touchDetected[pins[i]] = false;
    } else{
      Serial.print(0);
      Serial.print('\t');
    }
  
  }
  for(int i=8;i<10;i++){
    int val = touchRead(pins[i]);
      //touchDetected[pins[i]] = false;
      Serial.print(val);
      Serial.print('\t');
  }
  Serial.println('\t');
}