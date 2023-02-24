#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define LEFT_TILT 0
#define RIGHT_TILT 1

// The threshold value to overcome to be registered as a tilt
float tiltThreshold = 0.8;

// Int to store the number of LEDs
int numLED = 3;

// Array to store LED pin values
int LEDPins[] = {8, 9, 10};

// Int to store current LED index into array
int currentLED = 1;

// Create an object  from the Adafruit library to manage the mpu
Adafruit_MPU6050 mpu;

// The last time we got values from the MPU 6050
float lastReadTime;

// How many times do we want to read the accelerometer when calculating our calibration values
int numberCalibrationReads = 200;

// Our offset x and y values for the accelerometer that are calculated in setup
// Used for calibration and to remove wiggle
float accelerometerOffsetX = 0;
float accelerometerOffsetY = 0;

// Our offset x and y values for the gyroscope that are calculated in setup
// Used for calibration and to remove wiggle
float gyroscopeOffsetX = 0;
float gyroscopeOffsetY = 0;
float gyroscopeOffsetZ = 0;

// The calculated roll (i.e., x axis rotation) value using accelerometer and gyroscope data
float roll = 0;

// The calculated pitch (i.e., y axis rotation) value using accelerometer and gyroscope data
float pitch = 0;

// The calculated roll (i.e., z axis rotation) value using accelerometer and gyroscope data
float yaw = 0;

// ********************************************************************************
// ************************************ Set Up ************************************
// ********************************************************************************
void setup(void) {
	Serial.begin(9600);

	// Try to initialize
	if (!mpu.begin()) {
		Serial.println("Failed to find MPU6050 chip");
		while (1) {
		  delay(10);
		}
	}
	Serial.println("MPU6050 Found!");

	// Set the accelerometer range on the MPU 6050
  // This is the range of forces the accelerometer can measure
  // The smaller the range, the more sensitive the readings will be
  // Available ranges are:
  //   +/- 2G
  //   +/- 4G
  //   +/- 8G
  //   +/- 16G
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);

	// Set the gyroscope range on the MPU 6050
  // This is the degrees of rotation per second the gyroscope can measure
  // The smaller the range, the more sensitive the readings will be
  // Available ranges are:
  //   +/- 250 degrees per second
  //   +/- 500 degrees per second
  //   +/- 1000 degrees per second
  //   +/- 2000 degrees per second
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);

	// Set filter bandwidth to 21 Hz
	mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Set up the LEDS
  for (int i = 0; i < numLED; ++i) {
     pinMode(LEDPins[i], OUTPUT);
  }

  // Start with the middle LED on
  digitalWrite(LEDPins[currentLED], HIGH);

  // Calculate our resting values (i.e., offsets) for the accelerometer and gyroscope
  calculateIMUOffset();

  // Now that we have read from the MPU 6050 for calcualting offsets, store the current time as our last read time
  lastReadTime = millis();

	delay(100);
}

// ********************************************************************************
// ************************************ Loop ************************************
// ********************************************************************************
void loop() {
  // Variables to store the sensor values from our MPU 6050
  sensors_event_t a, g, temp;

  // Reads the accelerometer, gyroscope, and temperature sensor values from the MPU 6050  
  mpu.getEvent(&a, &g, &temp);

  // Now calculate how much time has elapsed in seconds since our last read from the MPU 6050
  float elapsedTimeSeconds = (millis() - lastReadTime) / 1000;

  // Update the lastReadTime to now since we just read from the MPU 6050
  lastReadTime = millis();

  // Calculate the x (roll) and y (pitch) orientation angles from the accelerometer data
  float accelerometerAngleX = (atan(a.acceleration.y / sqrt(pow(a.acceleration.x, 2) + pow(a.acceleration.z, 2))) * 180 / PI) - accelerometerOffsetX;
  float accelerometerAngleY = (atan(-1 * a.acceleration.x / sqrt(pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2))) * 180 / PI) - accelerometerOffsetY;

  // Calculate the calibrated gyroscope x, y, and z angles using the offset values we calculated in calculateIMUOffset()
  float gyroscopeAngleX = g.gyro.x - gyroscopeOffsetX;
  float gyroscopeAngleY = g.gyro.y - gyroscopeOffsetY;
  float gyroscopeAngleZ = g.gyro.z - gyroscopeOffsetZ;
  
  // Currently the raw values are in radians per second, so we need to multiply by sendonds to get the angle in radians
  gyroscopeAngleX *= elapsedTimeSeconds;
  gyroscopeAngleY *= elapsedTimeSeconds;

  // Combine acceleromter and gyroscope angle values using what is called a "complementary filter" to calculate current roll and pitch
  roll = -(0.96 * gyroscopeAngleX + 0.04 * accelerometerAngleX) / 2;
  pitch = (0.96 * gyroscopeAngleY + 0.04 * accelerometerAngleY) / 2;

  // We need another sensor that isn't on the MPU 6050 in order to calculate yaw the same way
  // So instead we are just using an approximation by adding the gyroscope z value every loop
  // This works because gyroscopes measure rotational velocity (i.e., rate of change of the angular position over time)
  // So we can just add up these changes over time to estimate where the z orientation is
  yaw += gyroscopeAngleZ * elapsedTimeSeconds;

  // Send the roll (x), pitch (y), and yaw(z) values over serial 
  // Serial.print("Roll: ");
  // Serial.print(roll);
  // Serial.print(", ");
  // Serial.print("Pitch: ");
  // Serial.print(pitch);
  // Serial.print(", ");
  // Serial.print("Yaw: ");
  // Serial.println(yaw);

  // Wait a fair amount of time before reading again, e.g., 100ms or more
  // If we read too frequently then everything will wobble as the components in the accelerometer are still in flux
  delay(150);

  // Detect a left tilt
  if (pitch > tiltThreshold) {
    Serial.println(LEFT_TILT);
    // If not ready at the leftmost place
    if (currentLED != 0) {
      // Turn off current LED
      digitalWrite(LEDPins[currentLED], LOW);
      // Decrement LED index
      --currentLED;
      // Turn on updated LED
      digitalWrite(LEDPins[currentLED], HIGH);
    }
    delay(30);
  }

  // Detect a right tilt
  if (pitch < (-tiltThreshold)) {
    Serial.println(RIGHT_TILT);
    // If not ready at the rightmost place
    if (currentLED != 2) {
      // Turn off current LED
      digitalWrite(LEDPins[currentLED], LOW);
      // Increment LED index
      ++currentLED;
      // Turn on updated LED
      digitalWrite(LEDPins[currentLED], HIGH);
    }
    delay(30);
  }
}

// This function will help calibrate our sensor reads to account for what the average accelerometer and gyroscope values are when it is sitting still
// Call during setup to calculate the accelerometer and gyroscope offsets
// NOTE: make sure to place the IMU flat in order to get the proper calibration offset values
void calculateIMUOffset() {
  // Variables to store the sensor values from our MPU 6050
  sensors_event_t a, g, temp;

  // Perform numberCalibrationReads from the MPU 6050 and add up the values from each read
  for(int i = 0; i < numberCalibrationReads; i++) {
    // Reads the accelerometer, gyroscope, and temperature sensor values from the MPU 6050
    mpu.getEvent(&a, &g, &temp);
    
    // Sum the accelerometer readings
    accelerometerOffsetX += ((atan((a.acceleration.y) / sqrt(pow((a.acceleration.x), 2) + pow((a.acceleration.z), 2))) * 180 / PI));
    accelerometerOffsetY += ((atan(-1 * (a.acceleration.x) / sqrt(pow((a.acceleration.y), 2) + pow((a.acceleration.z), 2))) * 180 / PI));

    // Sum the gyroscope readings
    gyroscopeOffsetX += (g.gyro.x);
    gyroscopeOffsetY += (g.gyro.y);
    gyroscopeOffsetZ += (g.gyro.z);
  }
  
  //Divide the sum total of each offset value by numberCalibrationReads to get the average offset value
  accelerometerOffsetX /= numberCalibrationReads;
  accelerometerOffsetY /= numberCalibrationReads;
  gyroscopeOffsetX /= numberCalibrationReads;
  gyroscopeOffsetY /= numberCalibrationReads;
  gyroscopeOffsetZ /= numberCalibrationReads ;
  
  // Uncomment the code below to print the offset calibration values to the Serial Monitor
  /*Serial.print("Accelerometer Offset X: ");
  Serial.println(accelerometerOffsetX);
  Serial.print("Accelerometer Offset Y: ");
  Serial.println(accelerometerOffsetY);
  Serial.print("Gyroscope Offset X: ");
  Serial.println(gyroscopeOffsetX);
  Serial.print("Gyroscope Offset Y: ");
  Serial.println(gyroscopeOffsetY);
  Serial.print("Gyroscope Offset Z: ");
  Serial.println(gyroscopeOffsetZ);*/
}