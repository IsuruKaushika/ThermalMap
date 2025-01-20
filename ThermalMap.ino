#include <AFMotor.h>          // For controlling stepper motors
#include <Wire.h>             // For I2C communication
#include <VL53L0X.h>          // For VL53L0X distance sensor
#include <Adafruit_MLX90614.h> // For MLX90614 temperature sensor

// Define stepper motors (200 steps per revolution)
AF_Stepper motorX(200, 2); // X-axis motor
AF_Stepper motorY(200, 1); // Y-axis motor

// Sensors
VL53L0X sensor;                   // Distance sensor
Adafruit_MLX90614 mlx = Adafruit_MLX90614(); // Temperature sensor

// Grid dimensions
const int maxX = 100; // X-axis limit
const int maxY = 50;  // Y-axis limit

// Movement settings
const int stepsPerPoint = 2;    // Steps between each measurement
const int delayBetweenSteps = 300; // Delay in ms between steps

// Limit switches
const int switchPin1 = 13; // Y-axis limit switch
const int switchPin2 = 2;  // X-axis limit switch

void setup() {
  Serial.begin(9600);
  Serial.println("Starting 2D Temperature and Distance Scan");

  // Initialize motors
  motorX.setSpeed(60);
  motorY.setSpeed(60);

  pinMode(A0, OUTPUT);

  // Initialize limit switches
  pinMode(switchPin1, INPUT_PULLUP);
  pinMode(switchPin2, INPUT_PULLUP);

  StartTone();

  // Initialize VL53L0X sensor
  Wire.begin();
  sensor.setTimeout(500);
  if (!sensor.init()) {
    Serial.println("Failed to initialize VL53L0X sensor!");
    while (1);
  }
  sensor.startContinuous();

  // Initialize MLX90614 sensor
  if (!mlx.begin()) {
    Serial.println("Failed to initialize MLX90614 sensor!");
    while (1);
  }

  // Homing motors
  homeMotors();

  Serial.println("Initialization complete. Starting scan...");
}

void loop() {
  for (int x = 0; x <= maxX; x++) {
    if (x % 2 == 0) {
      // Move forward along Y-axis
      for (int y = 0; y <= maxY; y++) {
        moveToPoint(x, y);
        readAndPrintSensors(x, y);
      }
    } else {
      // Move backward along Y-axis
      for (int y = maxY; y >= 0; y--) {
        moveToPoint(x, y);
        readAndPrintSensors(x, y);
      }
    }
  }

  Serial.println("Scan complete!");
  soundAlarm();
  while (true); // Stop execution after scan
}

void homeMotors() {
  Serial.println("Homing motors...");

  // Home Y-axis
  while (digitalRead(switchPin1) == HIGH) {
    motorY.step(1, BACKWARD, SINGLE);
    delay(10);
  }

  // Home X-axis
  while (digitalRead(switchPin2) == HIGH) {
    motorX.step(1, BACKWARD, SINGLE);
    delay(10);
  }

  Serial.println("Homing complete.");
}

void moveToPoint(int x, int y) {
  static int currentX = 0, currentY = 0;

  // Calculate steps for X-axis
  int xSteps = (x - currentX) * stepsPerPoint;
  if (xSteps > 0) motorX.step(xSteps, FORWARD, SINGLE);
  else if (xSteps < 0) motorX.step(-xSteps, BACKWARD, SINGLE);

  // Calculate steps for Y-axis
  int ySteps = (y - currentY) * stepsPerPoint;
  if (ySteps > 0) motorY.step(ySteps, FORWARD, SINGLE);
  else if (ySteps < 0) motorY.step(-ySteps, BACKWARD, SINGLE);

  // Update current position
  currentX = x;
  currentY = y;

  delay(delayBetweenSteps); // Optional delay
}

void readAndPrintSensors(int x, int y) {
  // Read distance from VL53L0X
  int distance = sensor.readRangeContinuousMillimeters();
  bool timeoutOccurred = sensor.timeoutOccurred();

  // Read temperatures from MLX90614
  float ambientTemp = mlx.readAmbientTempC();
  float objectTemp = mlx.readObjectTempC();

  // Print data
  //Serial.print("Point (");
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.print(",");
  //Serial.print("): ");
  if (!timeoutOccurred) {
    //Serial.print("Distance = ");
    Serial.print(distance);
    Serial.print(",");
    //Serial.print(",");
   // Serial.print(" mm, ");
  } else {
    //Serial.print("Distance = TIMEOUT, ");
  }
  //Serial.print("Ambient Temp = ");
 // Serial.print(ambientTemp);
  //Serial.print(",");
  //Serial.print(" °C, ");
  //Serial.print("Object Temp = ");
  Serial.print(objectTemp);
   Serial.print(",");
  Serial.println(ambientTemp);
}
void soundAlarm() {
  for (int i = 0; i < 6; i++) { // Repeat 6 times (6 * 500ms = 3000ms)
    tone(A0, 1000);  // Generate a 1000 Hz tone
    delay(500);      // Keep it on for 500ms
    noTone(A0);      // Stop the tone briefly
    delay(50);       // Add a short pause (optional)
  }
}

void StartTone() {
  for (int i = 0; i < 2; i++) { // Repeat 6 times (6 * 500ms = 3000ms)
    tone(A0, 800);  // Generate a 1000 Hz tone
    delay(500);      // Keep it on for 500ms
    noTone(A0);      // Stop the tone briefly
    delay(50);       // Add a short pause (optional)
  }
}
