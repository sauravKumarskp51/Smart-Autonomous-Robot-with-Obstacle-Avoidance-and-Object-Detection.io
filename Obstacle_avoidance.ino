#include <Arduino.h>

// Define control pins for motors
#define MOTOR1_IN1 25  // Left Motor Direction 1
#define MOTOR1_IN2 26  // Left Motor Direction 2
#define MOTOR2_IN1 27  // Right Motor Direction 1
#define MOTOR2_IN2 14  // Right Motor Direction 2

// Define PWM pins for speed control
#define MOTOR1_PWM 21  // Left Motor Speed (ENA)
#define MOTOR2_PWM 22  // Right Motor Speed (ENB)

// Define ultrasonic sensor pins
#define TRIG_MIDDLE 19
#define ECHO_MIDDLE 23
#define TRIG_LEFT 5
#define ECHO_LEFT 18
#define TRIG_RIGHT 15
#define ECHO_RIGHT 4
#define TRIG_FULLY_LEFT 12
#define ECHO_FULLY_LEFT 13
#define TRIG_FULLY_RIGHT 32
#define ECHO_FULLY_RIGHT 33

// PWM configuration
#define FREQUENCY 5000  // PWM frequency in Hz
#define RESOLUTION 8    // PWM resolution (1-16 bits)
#define SPEED 255       // Default motor speed (0-255)
#define OBSTACLE_DISTANCE 25 // Threshold distance in cm

// Variables for sensor distances
float middleDistance = 0, leftDistance = 0, rightDistance = 0;
float fullyLeftDistance = 0, fullyRightDistance = 0;

// Timing for non-blocking measurement
unsigned long lastMeasurementTime = 0;
const unsigned long measurementInterval = 50; // Measure every 50 ms

void setup() {
  // Motor setup
  pinMode(MOTOR1_IN1, OUTPUT);
  pinMode(MOTOR1_IN2, OUTPUT);
  pinMode(MOTOR2_IN1, OUTPUT);
  pinMode(MOTOR2_IN2, OUTPUT);

  // Attach PWM pins for speed control
  ledcAttach(MOTOR1_PWM, FREQUENCY, RESOLUTION);
  ledcAttach(MOTOR2_PWM, FREQUENCY, RESOLUTION);

  // Ultrasonic sensor setup
  pinMode(TRIG_MIDDLE, OUTPUT);
  pinMode(ECHO_MIDDLE, INPUT);
  pinMode(TRIG_LEFT, OUTPUT);
  pinMode(ECHO_LEFT, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);
  pinMode(TRIG_FULLY_LEFT, OUTPUT);
  pinMode(ECHO_FULLY_LEFT, INPUT);
  pinMode(TRIG_FULLY_RIGHT, OUTPUT);
  pinMode(ECHO_FULLY_RIGHT, INPUT);

  Serial.begin(9600); // For debugging
}

// Function to measure distance using ultrasonic sensor
float measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // Timeout after 30ms
  if (duration == 0) return 999; // No echo received
  return (duration * 0.034) / 2; // Convert to cm
}

// Function to move forward
void moveForward(int speed) {
  digitalWrite(MOTOR1_IN1, HIGH);
  digitalWrite(MOTOR1_IN2, LOW);
  digitalWrite(MOTOR2_IN1, HIGH);
  digitalWrite(MOTOR2_IN2, LOW);
  ledcWrite(MOTOR1_PWM, speed);
  ledcWrite(MOTOR2_PWM, speed);
}
// Function to move backward
void moveBackward(int speed) {
  digitalWrite(MOTOR1_IN1, LOW);
  digitalWrite(MOTOR1_IN2, HIGH);
  digitalWrite(MOTOR2_IN1, LOW);
  digitalWrite(MOTOR2_IN2, HIGH);
  ledcWrite(MOTOR1_PWM, speed);
  ledcWrite(MOTOR2_PWM, speed);
}
// Function to turn left
void turnLeft(int speed) {
  digitalWrite(MOTOR1_IN1, LOW);
  digitalWrite(MOTOR1_IN2, HIGH);
  digitalWrite(MOTOR2_IN1, HIGH);
  digitalWrite(MOTOR2_IN2, LOW);
  ledcWrite(MOTOR1_PWM, speed);
  ledcWrite(MOTOR2_PWM, speed);
}

// Function to turn right
void turnRight(int speed) {
  digitalWrite(MOTOR1_IN1, HIGH);
  digitalWrite(MOTOR1_IN2, LOW);
  digitalWrite(MOTOR2_IN1, LOW);
  digitalWrite(MOTOR2_IN2, HIGH);
  ledcWrite(MOTOR1_PWM, speed);
  ledcWrite(MOTOR2_PWM, speed);
}

// Function to stop the motors
void stopMotors() {
  ledcWrite(MOTOR1_PWM, 0);
  ledcWrite(MOTOR2_PWM, 0);
}

void updateDistances() {
  middleDistance = measureDistance(TRIG_MIDDLE, ECHO_MIDDLE);
  leftDistance = measureDistance(TRIG_LEFT, ECHO_LEFT);
  rightDistance = measureDistance(TRIG_RIGHT, ECHO_RIGHT);
  fullyLeftDistance = measureDistance(TRIG_FULLY_LEFT, ECHO_FULLY_LEFT);
  fullyRightDistance = measureDistance(TRIG_FULLY_RIGHT, ECHO_FULLY_RIGHT);

  // Debugging distances
  Serial.print("Middle: ");
  Serial.print(middleDistance);
  Serial.print(" cm, Left: ");
  Serial.print(leftDistance);
  Serial.print(" cm, Right: ");
  Serial.print(rightDistance);
  Serial.print(" cm, Fully Left: ");
  Serial.print(fullyLeftDistance);
  Serial.print(" cm, Fully Right: ");
  Serial.println(fullyRightDistance);
}

void loop() {
  // Non-blocking distance measurement
  unsigned long currentTime = millis();
  if (currentTime - lastMeasurementTime >= measurementInterval) {
    lastMeasurementTime = currentTime;
    updateDistances();
  }

  // Main behavior logic
  if (fullyLeftDistance < 30) {
    turnRight(SPEED);
    delay(100);
    stopMotors();
    moveForward(SPEED);
    stopMotors();
  } else if (fullyRightDistance < 30) {
    turnLeft(SPEED);
    delay(100);
    stopMotors();
    moveForward(SPEED);
    stopMotors();
  } else if (middleDistance > OBSTACLE_DISTANCE && leftDistance > OBSTACLE_DISTANCE && rightDistance > OBSTACLE_DISTANCE) {
    moveForward(SPEED);
  } else if (middleDistance < OBSTACLE_DISTANCE) {
    stopMotors();
    if (leftDistance > rightDistance) {
      turnLeft(SPEED);
      delay(100);
    } else {
      turnRight(SPEED);
      delay(100);
    }
    stopMotors();
  } else if (leftDistance < OBSTACLE_DISTANCE) {
    turnRight(SPEED);
    delay(200);
    stopMotors();
  } else if (rightDistance < OBSTACLE_DISTANCE) {
    turnLeft(SPEED);
    delay(200);
    stopMotors();
  }

  // Check for being stuck in a corner
  if (middleDistance < 20 && leftDistance < 20 && rightDistance < 20 && fullyLeftDistance < 20 && fullyRightDistance < 20) {
    stopMotors();
    moveBackward(SPEED);
    delay(200);
    stopMotors();
  }


  delay(10); // Small delay for stability
}
