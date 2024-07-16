/*
Handles IMU input and sends MIDI data
*/

const byte IMU_DEBUG = 0; //enable to see data in arduino console
const int IMU_DATA_RANGE = 10000; //the maximum accerlation value we are interested in
const byte changeThreshold = 3;
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
  static int32_t onepole[] = {0,0,0};
  static int count = 0;
  static int prev_output[3] = {0,0,0};

  //every 5 milliseconds read the imu data
  // accumulate these readings
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

  //every 50 ms seconds the average of the imu data, 
  //and clear the accumulated values
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
      int32_t val[3] = {0,0,0};
      for(int j=0;j<3;j++) {
        val[j] = accel[j]/count;
        val[j] = val[j]*.25 + onepole[j]*.75;
        onepole[j] = val[j];
      }

      int32_t output = 0;
      if(output > IMU_DATA_RANGE) output = IMU_DATA_RANGE;
      else if (output < -IMU_DATA_RANGE) output = -IMU_DATA_RANGE;
      output = map(val[0], -IMU_DATA_RANGE, IMU_DATA_RANGE, 0, 127);
      if(abs(output-prev_output[0])){
        prev_output[0] = output;
        sendMidiCC(accelCCs[0], (byte) output);
      }
      
      if(output > IMU_DATA_RANGE) output = IMU_DATA_RANGE;
      else if (output < -IMU_DATA_RANGE) output = -IMU_DATA_RANGE;
      output = map(val[1], -IMU_DATA_RANGE, IMU_DATA_RANGE, 0, 127);
      if(abs(output-prev_output[1])){
        prev_output[1] = output;
        sendMidiCC(accelCCs[1], (byte) output);
      }

      if(output > IMU_DATA_RANGE) output = IMU_DATA_RANGE;
      else if (output < -IMU_DATA_RANGE) output = -IMU_DATA_RANGE;
      output = map(val[2], -IMU_DATA_RANGE, IMU_DATA_RANGE, 0, 127);
      if(abs(output-prev_output[2])){
        prev_output[2] = output;
        sendMidiCC(accelCCs[2], (byte) output);
      }

      for( int i=0;i<3;i++){
        accel[i] = 0;
        gyro[i] = 0;
      }
      count = 0;
    }
  }
}