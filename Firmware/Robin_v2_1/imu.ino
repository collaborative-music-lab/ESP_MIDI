/*float accelX, accelY, accelZ; // Replace with real IMU readings

float flickThreshold = 1;   // Gs or unit matching your IMU
uint8_t flashBrightness = 0;
// Sensitivity settings (adjust for your IMU and preference)
const float FLICK_THRESHOLD = 0.5; // in g
const uint8_t DECAY_RATE = 20;      // how fast to fade (higher = faster)
uint8_t buttonState[5];
uint8_t buttonIsPressed = 0;

// State variable
uint8_t gradientBrightness = 0; // Current max brightness of tip

uint8_t motionLevel = 0; // 0–255
const uint8_t LEAK_AMOUNT = 25;   // how fast it fades (1–10)
const uint8_t INPUT_GAIN = 255; // scale input motion (adjust as needed)
const float MOTION_SCALE = 1.0; // scale motionMagnitude into 0–1



//IMU DATA PROCESSING

SensorTriple accel;
SensorTriple gyro;


SmoothedIMU smoothedIMU = {0};
const float smoothingAlpha = 0.1;  // Lower = more smoothing

struct TiltAngles {
  float roll;   // degrees
  float pitch;  // degrees
  float yaw;    // degrees (approximate from tilt)
};
float tiltScaling = 1;
float magnitudeScaling = 1;

struct GyroAngle {
  float x = 0;
  float y = 0;
  float z = 0;
};

GyroAngle integratedGyro;
const float decayFactor = 0.995;  // Closer to 1 = less leak
unsigned long lastTime = 0;

*/

float accelRest = 0;
struct SmoothedIMU {
  float accelX, accelY, accelZ;
  float gyroX, gyroY, gyroZ;
};
int axRaw, ayRaw, azRaw;         // raw gyro values
int gxRaw, gyRaw, gzRaw;         // raw gyro values

float imuPrev[3] = {0,0,0};
float imuFiltered[3] = {0,0,0};
float magnitudePrev = 0;

float tiltVals[3] = {0,0,0};

void imuSetup(){
  BMI160.begin(BMI160GenClass::I2C_MODE);
  uint8_t dev_id = BMI160.getDeviceID();
  // while (!imu.init())
  // {
  //   Serial.println("Failed to detect and initialize IMU!");
  //   delay(1000);
  // }
  // imu.enableDefault();
  delay(100);
  readImuAccel();
  accelRest = calcTotalMagnitude(axRaw, ayRaw, azRaw);
  if(SERIAL_DEBUG) Serial.println("imu setup");
}

char report[80];

void imuLoop(){
  static uint32_t timer = 0;
  int interval = 20; 

  if(millis()-timer > interval){
    //statusLed(1);
    timer= millis();

    readImuAccel();
    calcTilt(axRaw, ayRaw, azRaw);

    float imuNew[] = {axRaw, ayRaw, azRaw};
    float leak = 0.9;

    for(byte i=0;i<3;i++){
      float delta = imuPrev[i] - imuNew[i];
      imuPrev[i] = imuNew[i];

      imuFiltered[i] = imuFiltered[i] * leak +  delta;
      
      cc[5+i].send( int((imuFiltered[i]) / 200) ) ;
      // Serial.print(abs(  int(imuFiltered[i] / 500) ));
      // Serial.print("\t");
    }

      // Serial.print(abs(imuFiltered[0]/100));
      // Serial.print("\t");
      // Serial.print(abs(imuFiltered[1]/100));
      // Serial.print("\t");
      // Serial.println(abs(imuFiltered[2]/100));
      // return;

    for(byte i=0;i<3;i++){
      cc[8+i].send( int(tiltVals[i]) );
      // Serial.print( int(tiltVals[i]));
      // Serial.print("\t");
    }

    float mag = calcTotalMagnitude(imuFiltered[0], imuFiltered[1], imuFiltered[2]);
    mag = constrain( mag*20, 0, 127);
    cc[11].send(mag);
    //Serial.println(abs(  mag) );
    

      // snprintf(report, sizeof(report), "A: %6d %6d %6d    G: %6d %6d %6d",
      //   imu.a.x, imu.a.y, imu.a.z,
      //   imu.g.x, imu.g.y, imu.g.z);
      // Serial.println(report);

  }
}

void readImuAccel(){
  BMI160.readAccelerometer(axRaw, ayRaw, azRaw);
}
void readImuGyro(){
  BMI160.readGyro(gxRaw, gyRaw, gzRaw);
}

float calcTotalMagnitude(float x, float y, float z){
  // sensitivity = 0.000061 for ±2g (default)
  // use 0.000122 for ±4g, etc.
  const float ACCEL_SENSITIVITY = 0.000061;

  float ax_g = x * ACCEL_SENSITIVITY;
  float ay_g = y * ACCEL_SENSITIVITY;
  float az_g = z * ACCEL_SENSITIVITY;
  float leak = 0.8;

  float mag = magnitudePrev*leak + sqrt(ax_g * ax_g + ay_g * ay_g + az_g * az_g);
  magnitudePrev = mag;

  return mag;
}

// float calcMagnitude(float axis){
//   // sensitivity = 0.000061 for ±2g (default)
//   // use 0.000122 for ±4g, etc.
//   const float ACCEL_SENSITIVITY = 0.000061;

//   float ax_g = x * ACCEL_SENSITIVITY;
//   float ay_g = y * ACCEL_SENSITIVITY;
//   float az_g = z * ACCEL_SENSITIVITY;

//   return sqrt(ax_g * ax_g + ay_g * ay_g + az_g * az_g) - accelRest;
// }

void calcTilt(float x, float y, float z) {
  float norm = sqrt(x * x + y * y + z * z);
  if (norm == 0) return;

  // normalize gravity vector
  x /= norm;
  y /= norm;
  z /= norm;

  // angle between gravity and each axis in degrees
  float tiltX = acos(x) * 180.0 / PI;  // how "vertical" X is
  float tiltY = acos(y) * 180.0 / PI;  // how "vertical" Y is
  float tiltZ = acos(z) * 180.0 / PI;  // how "vertical" Z is

  tiltVals[0] = tiltX;
  tiltVals[1] = tiltY;
  tiltVals[2] = tiltZ;
  // Serial.print(tiltX);
  // Serial.print("\t");
  // Serial.print(tiltY);
  // Serial.print("\t");
  // Serial.println(tiltZ);
}