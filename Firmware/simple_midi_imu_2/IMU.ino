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
      Serial.print("imu " + String(imu.a.x) + "\t" + String(imu.a.y) + "\t" + String(imu.a.z));
      Serial.println(String(imu.g.x) + "\t" + String(imu.g.y) + "\t" + String(imu.g.z));
    }
    else{
      //send each axis of acceleration independently
      int val = imu.a.x;
      if(val > 1000) val = 1000;
      else if (val < -1000) val = -1000;
      sendMidiCC(accelCCs[0], map(val, -1000, 1000, 0, 127));

      val = imu.a.y;
      if(val > 1000) val = 1000;
      else if (val < -1000) val = -1000;
      sendMidiCC(accelCCs[1], map(val, -1000, 1000, 0, 127));

      val = imu.a.z;
      if(val > 1000) val = 1000;
      else if (val < -1000) val = -1000;
      sendMidiCC(accelCCs[2], map(val, -1000, 1000, 0, 127));

      //send the total magnitude of acceleration
      val = sqrt(pow(imu.a.x,2) + pow(imu.a.y,2) + pow(imu.a.z,2));
      if(val > 1000) val = 1000;
      sendMidiCC(accelCCs[2], map(val, 0, 1000, 0, 127));

      for( int i=0;i<3;i++){
        accel[i] = 0;
        gyro[i] = 0;
      }
      count = 0;
    }
  }
}