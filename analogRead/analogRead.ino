#include "driver/gpio.h"

void setup() {
  Serial.begin(115200); // Initialize serial communication at 9600 baud rate
  for (int i = 0; i < 10; i++) {
     gpio_config_t io_conf;
      io_conf.intr_type = GPIO_INTR_DISABLE;
      io_conf.mode = GPIO_MODE_INPUT;
      io_conf.pin_bit_mask = (1ULL << i);
      io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
      io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
      gpio_config(&io_conf);
    //pinMode(i, INPUT_PULLUP);
  }
}

void loop() {
  // Array to hold the values
  int values[14];

  // Read analog values from pins A0 to A5
  for (int i = 0; i < 10; i++) {
    values[i] = analogRead(i+1);
  }


  // Print all values on a single line
  for (int i = 0; i < 10; i++) {
    Serial.print(values[i]);
    if (i < 13) {
      Serial.print("\t");
    }
  }
  Serial.println(); // Move to the next line

  delay(50); // Wait for 50 milliseconds
}
