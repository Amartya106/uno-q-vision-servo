#include <Servo.h>
#include <Arduino_LED_Matrix.h>
#include <Arduino_RouterBridge.h>

Servo myServo;
Arduino_LED_Matrix matrix;

const int servoPin = 10;
const int initialAngle = 90;
const int minAngle = 45;
const int maxAngle = 135;

enum State { IDLE, WAVING };
State currentState = IDLE;

int waveCount = 0;
int angle = initialAngle;
bool directionUp = true;
unsigned long lastMove = 0;
const int moveInterval = 3; // ms

const uint32_t happy_face[]{
  0x000200a8,0x0a802000,0x04403e00,0x00000000
};

const uint32_t neutral_face[]{
  0x00000038,0x0e000000,0x00007f00,0x00000000
};

void say_hello() {
  currentState = WAVING;
  waveCount = 0;
  angle = initialAngle;
  directionUp = true;
  matrix.loadFrame(happy_face);
  digitalWrite(LED_BUILTIN, HIGH);
}

void show_neutral() {
  currentState = IDLE;
  matrix.loadFrame(neutral_face);
  digitalWrite(LED_BUILTIN, LOW);
  myServo.write(initialAngle);
}

void reset_state() {
  currentState = IDLE;
  matrix.loadFrame(neutral_face);
  digitalWrite(LED_BUILTIN, LOW);
  myServo.write(initialAngle);
}

void setup() {
  myServo.attach(servoPin);
  myServo.write(initialAngle);

  matrix.begin();
  matrix.loadFrame(neutral_face);

  pinMode(LED_BUILTIN, OUTPUT);

  Bridge.begin();
  Bridge.provide("say_hello", say_hello);
  Bridge.provide("show_neutral", show_neutral);
  Bridge.provide("reset_state", reset_state);
}

void loop() {
  if (currentState == WAVING) {
    unsigned long now = millis();

    if (now - lastMove >= moveInterval) {
      lastMove = now;

      if (directionUp) angle++;
      else angle--;

      myServo.write(angle);

      if (angle >= maxAngle) directionUp = false;
      if (angle <= minAngle) {
        directionUp = true;
        waveCount++;
      }

      if (waveCount >= 3) {
        currentState = IDLE;
        myServo.write(initialAngle);
        digitalWrite(LED_BUILTIN, LOW);
        matrix.loadFrame(happy_face);
      }
    }
  }
}