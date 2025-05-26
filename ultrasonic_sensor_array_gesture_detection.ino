#include "SR04.h"
// SR04 library at: 
// https://github.com/mrRobot62/Arduino-ultrasonic-SR04-library/tree/master

// Pin definitions
#define TRIG_PIN_LEFT   2
#define ECHO_PIN_LEFT   3

#define TRIG_PIN_MID    4
#define ECHO_PIN_MID    5

#define TRIG_PIN_RIGHT  6
#define ECHO_PIN_RIGHT  7

// Sensor configuration
const float sensorSpacing = 5.0;  // Distance between adjacent sensors (cm)
const float d = sensorSpacing;
const int echoDelay = 10;
const int cycleDelay = 40;
const int loopDelay = max(20, cycleDelay - 2*echoDelay);
const int maxTrackLostIgnoreCnt = 1;
const int maxDist = 200;

// Hysteresis tracking config
const long max_z = 25;        // Detection threshold (cm)
const long hysteresis = 5;    // Hysteresis range (cm)
int trackLostCnt = 0;

// Tracking state definition
enum TrackingState { NO_OBJECT, ACQUIRED, TRACKING, LOST };
TrackingState currentState = NO_OBJECT;

// gesture config
const unsigned long min_gesture_duration = 150;      // in milliseconds
const unsigned long max_tap_duration = 800;          // max for tap vs tap-hold
const float min_swipe_distance = 1.5;                // cm threshold for swipe

// Gesture enum type
enum GestureType { NO_GESTURE, TAP, TAP_HOLD, SWIPE_LEFT, SWIPE_RIGHT };

// Sensor objects
SR04 sr04_left(ECHO_PIN_LEFT, TRIG_PIN_LEFT);
SR04 sr04_mid(ECHO_PIN_MID, TRIG_PIN_MID);
SR04 sr04_right(ECHO_PIN_RIGHT, TRIG_PIN_RIGHT);

// Global variables
long dist_left, dist_mid, dist_right;
float tracked_x = 0;
long tracked_z = 0;

// Global gesture tracking variables
float x_acquired = 0;
float x_lost = 0;
unsigned long t_acquired = 0;
unsigned long t_lost = 0;


void printDistances() {
  Serial.print("Left: ");
  Serial.print(dist_left);
  Serial.print(" cm, ");

  Serial.print("Middle: ");
  Serial.print(dist_mid);
  Serial.print(" cm, ");

  Serial.print("Right: ");
  Serial.print(dist_right);
  Serial.println(" cm");
}

// Prints gesture detection output
void printGesture(GestureType gesture) {
  switch (gesture) {
    case NO_GESTURE:
      Serial.println("No gesture: too brief.");
      break;
    case TAP:
      Serial.println("Gesture: Tap");
      break;
    case TAP_HOLD:
      Serial.println("Gesture: Tap and Hold");
      break;
    case SWIPE_LEFT:
      Serial.println("Gesture: Swipe Left");
      break;
    case SWIPE_RIGHT:
      Serial.println("Gesture: Swipe Right");
      break;
  }
}

// Prints information based on tracking state
void printTrackingState() {
  switch (currentState) {
    case NO_OBJECT:
      Serial.println("No object in range.");
      break;
    case ACQUIRED:
      Serial.print("Object acquired -> X: ");
      Serial.print(tracked_x);
      Serial.print(" cm, Z: ");
      Serial.print(tracked_z);
      Serial.println(" cm");
      break;
    case TRACKING:
      Serial.print("Tracking object -> X: ");
      Serial.print(tracked_x);
      Serial.print(" cm, Z: ");
      Serial.print(tracked_z);
      Serial.println(" cm");
      break;
    case LOST:
      Serial.println("Object lost.");
      break;
  }
}

void measureDistances() {
  dist_left = sr04_left.Distance();
  dist_left = min(dist_left, maxDist);
  delay(echoDelay);  // delay to minimize interference between sensors
  dist_mid = sr04_mid.Distance();
  dist_mid = min(dist_mid, maxDist);
  delay(echoDelay);
  dist_right = sr04_right.Distance();
  dist_right = min(dist_right, maxDist);
}

float estimateXPosition() {
  float invL = 1.0 / dist_left;
  float invM = 1.0 / dist_mid;
  float invR = 1.0 / dist_right;

  float sum = invL + invM + invR;
  if (sum == 0) return 0;

  float x = (-d * invL + 0 * invM + d * invR) / sum;
  return x;
}

long estimateZPosition() {
  return min(dist_left, min(dist_mid, dist_right));
}

// Detects gesture type based on position and time deltas
GestureType detectGesture() {
  unsigned long duration = t_lost - t_acquired;
  float delta_x = x_lost - x_acquired;
  float abs_dx = abs(delta_x);

  if (duration < min_gesture_duration) {
    return NO_GESTURE;
  }

  if (abs_dx < min_swipe_distance) {
    if (duration <= max_tap_duration) {
      return TAP;
    } else {
      return TAP_HOLD;
    }
  } else {
    return (delta_x > 0) ? SWIPE_RIGHT : SWIPE_LEFT;
  }
}

// Updates tracking state and object coordinates
void trackObjectState(float x, long z) {
  switch (currentState) {
    case NO_OBJECT:
    case LOST:
      trackLostCnt++;
      if (z < (max_z - hysteresis)) {
        currentState = ACQUIRED;
        tracked_x = x;
        tracked_z = z;
        x_acquired = x;
        t_acquired = millis();
        trackLostCnt = 0;
      }
      break;

    case ACQUIRED:
    case TRACKING:
      if (z > (max_z + hysteresis)) {
        currentState = LOST;
        x_lost = tracked_x;
        t_lost = millis();
        trackLostCnt++;
      } else {
        currentState = TRACKING;
        tracked_x = x;
        tracked_z = z;
        trackLostCnt = 0;
      }
      break;
  }
  if(trackLostCnt == maxTrackLostIgnoreCnt + 1) {
     GestureType gesture = detectGesture();
     printGesture(gesture);
  }
  else if (trackLostCnt > 2*maxTrackLostIgnoreCnt + 1)
    trackLostCnt--;
}

void setup() {
  Serial.begin(9600);
  delay(1000);
}

void loop() {
  measureDistances();
  //printDistances();  // Optional: Uncomment to debug raw sensor data

  float xPos = estimateXPosition();
  long zPos = estimateZPosition();

  trackObjectState(xPos, zPos);
  printTrackingState();

  delay(loopDelay);
}
