const byte IMU_DEBUG = 0;

void imuSetup(){
  Wire.begin(44,43);

  if (!imu.init())
  {
    Serial.println("Failed to detect and initialize IMU!");
    //while (1);
  }
  imu.enableDefault();
  imu.setTimeout(100);
   Serial.println("imu setup");
}

void imuLoop(){
  static uint32_t readTimer = 0;
  int readInterval = 5;
  static int32_t accel[] = {0,0,0};
  static int32_t gyro[] = {0,0,0};
  static int count = 0;

  if(millis()-readTimer>readInterval){
    readTimer=millis();
    
    imu.read();

    accel[0]+= imu.a.x;
    accel[1]+= imu.a.y;
    accel[2]+= imu.a.z;
    gyro[0]+= imu.g.x;
    gyro[1]+= imu.g.y;
    gyro[2]+= imu.g.z;
    count++;
  }

  static uint32_t sendTimer = 0;
  int interval = 50;

  if(millis()-sendTimer>interval){
    sendTimer=millis();

    if(IMU_DEBUG){
      Serial.print("imu ");
      Serial.print(imu.a.x);
      Serial.print(" ");
      Serial.print(imu.a.y);
      Serial.print(" ");
      Serial.print(imu.a.z);
      Serial.print(" ");
  
      Serial.print(imu.g.x);
      Serial.print(" ");
      Serial.print(imu.g.y);
      Serial.print(" ");
      Serial.print(imu.g.z);
      Serial.println(" ");
    }
    else{
//       comms.outu8(71);
//       comms.out16(accel[0]/count);
//       comms.out16(accel[1]/count);
//       comms.out16(accel[2]/count);
//       comms.send();
// //        Serial.println("uimu");
// //
//       comms.outu8(72);
//       comms.out16(gyro[0]/count);
//       comms.out16(gyro[1]/count);
//       comms.out16(gyro[2]/count);
//       comms.send();

      for( int i=0;i<3;i++){
        accel[i] = 0;
        gyro[i] = 0;
      }
      count = 0;
    }
  }
}