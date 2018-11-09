#include "pitches.h"   // note definitions for playing MUSIC
#include "mCore.h"     // mcore

// constant definitions
#define ULTRASONIC_TIMEOUT 30000  // timeout for pulseIn
#define ULTRASONIC_THRESHOLD 300  // "close enough to wall to turn"
#define INFRARED_THRESHOLD_L 550  // "close enough to wall to adjust to left"
#define INFRARED_THRESHOLD_R 550  // "close enough to wall to adjust to right"
#define MIC_THRESHOLD 200         // "loud enough to be considered not noise"
#define MIC_DECIDE 200            // threshold value to decide mic loudness
#define MAX_SPEED 255             // max speed of motors
#define ADJUSTMENT_SPEED 200      // speed to use when adjusting direction
#define ADJUSTMENT_DELAY 1000     // time to stay at adjustment speed
#define LED_DELAY 30              // response time of LDR
#define TURNING_SPEED 255         //
#define TURN_DURATION 280         //
#define FORWARD_INTERVAL 1000     //
#define MUSIC_SPEED 64            // default 64

// pin definitions
// Port 1 contains 2 digital pins 11 and 12
// Port 2 contains 2 digital pins 9 and 10
// Port 3 contains 2 analog pins A2 and A3
// Port 4 contains 2 analog pins A0 and A1
#define ULTRASONIC 10  // ultrasonic sensor
#define LINE 11        // left line sensor
#define LIGHT A6       // light sensor
#define LED 13         // top led
#define IR_L A1        // left infrared sensor
#define IR_R A2        // right infrared sensor
#define MIC_LOW A3     // low pass mic output
#define MIC_HIGH A0    // high pass mic output
#define MOTOR_L M2     // left motor
#define MOTOR_R M1     // right motor
#define BUZZER 8       // buzzer
#define BUTTON A7      // button

// assign classes
MeDCMotor motor_l(MOTOR_L);
MeDCMotor motor_r(MOTOR_R);
MeRGBLed rgbled(LED);
MeBuzzer buzzer(BUZZER);

struct color {
    int r;
    int g;
    int b;
};

// sensor functions

/**
 * Reads the analog output of a pin 5 times, and returns the average value
 * rounded down to the nearest integer. 
 * 
 * @param[in] pin  The pin to read
 * @return         The average value of 5 readings from the pin
 */
int analogAvgRead(int pin) {
  int average = 0;
  for (int i = 0; i < 5; i += 1) {
    average += analogRead(pin);
  }
  return average / 5; // rounds down but negligible for our purposes
}

/**
 * Sends an ultrasonic pulse and waits for the echo. The timeout is defined
 * by ULTRASONIC_TIMEOUT. Returns the time in microseconds before the echo is
 * heard. Consider replacing with boolean function for "close enough to wall"
 * to save time and space? 
 * 
 * @return  Time taken for the ultrasonic pulse to return in microseconds.
 */
long read_ultrasonic_sensor() {
  pinMode(ULTRASONIC, OUTPUT);
  digitalWrite(ULTRASONIC, HIGH);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC, LOW);
  pinMode(ULTRASONIC, INPUT);
  return pulseIn(ULTRASONIC, HIGH, ULTRASONIC_TIMEOUT);
}

struct color read_ldr_sensor() {
  struct color result;
  rgbled.setColor(255, 0, 0);
  delay(LED_DELAY);
  result.r = analogRead(LIGHT);
  rgbled.setColor(0, 255, 0);
  delay(LED_DELAY);
  result.g = analogRead(LIGHT);
  rgbled.setColor(0, 0, 255);
  delay(LED_DELAY);
  result.b = analogRead(LIGHT);
  rgbled.clear();
  return result;
}

// motor functions

void adjust_to_left() {
  motor_r.run(MAX_SPEED);
  motor_l.run(-ADJUSTMENT_SPEED);
  return;
}

void adjust_to_right() {
  motor_r.run(ADJUSTMENT_SPEED);
  motor_l.run(-MAX_SPEED);
  return;
}

void turn_left() {
  motor_r.run(TURNING_SPEED);
  motor_l.run(TURNING_SPEED);
  delay(TURN_DURATION);
  motor_r.stop();
  motor_l.stop();
  return;
}

void turn_right() {
  motor_r.run(-TURNING_SPEED);
  motor_l.run(-TURNING_SPEED);
  delay(TURN_DURATION);
  motor_r.stop();
  motor_l.stop();
  return;
}

void turn_180() {
  turn_right();
  turn_right();
  return;
}

void move_forward() {    
  if (analogAvgRead(IR_L) < INFRARED_THRESHOLD_L) {
    adjust_to_right();
  } else if (analogAvgRead(IR_R) < INFRARED_THRESHOLD_R) {
    adjust_to_left();
  } else {
    motor_r.run(MAX_SPEED);
    motor_l.run(-MAX_SPEED);
  }
  return;
}

// TODO: Review below code
void turn_left_forward_left() {
  turn_left();
    move_forward();
  // while ultrasonic larger than value, keep moving forward
  while (read_ultrasonic_sensor() > ULTRASONIC_THRESHOLD) {
    delay(50);
  }
  turn_left();
  return;
}

void turn_right_forward_right() {
  turn_right();
  move_forward();
  // while ultrasonic larger than value, keep moving forward
  while (read_ultrasonic_sensor() > ULTRASONIC_THRESHOLD) {
    delay(50);
  }
  turn_right();
  return;
}

// victory theme
void start_tune() {
  buzzer.tone(NOTE_C6, 4000/MUSIC_SPEED);
  delay(4000/MUSIC_SPEED);
  buzzer.tone(NOTE_C6, 4000/MUSIC_SPEED);
  delay(4000/MUSIC_SPEED);
  buzzer.tone(NOTE_C6, 4000/MUSIC_SPEED);
  delay(4000/MUSIC_SPEED);
  buzzer.tone(NOTE_C6, 10000/MUSIC_SPEED);
  delay(12000/MUSIC_SPEED);
  buzzer.tone(NOTE_GS5, 10000/MUSIC_SPEED);
  delay(12000/MUSIC_SPEED);
  buzzer.tone(NOTE_AS5, 10000/MUSIC_SPEED);
  delay(12000/MUSIC_SPEED);
  buzzer.tone(NOTE_C6, 6000/MUSIC_SPEED);
  delay(8000/MUSIC_SPEED);
  buzzer.tone(NOTE_AS5, 4000/MUSIC_SPEED);
  delay(4000/MUSIC_SPEED);
  buzzer.tone(NOTE_C6, 30000/MUSIC_SPEED);
  delay(32000/MUSIC_SPEED);
  return;
}

void loop_tune() {
  buzzer.tone(NOTE_G5, 10000/MUSIC_SPEED);
  delay(12000/MUSIC_SPEED);
  buzzer.tone(NOTE_F5, 10000/MUSIC_SPEED);
  delay(12000/MUSIC_SPEED);
  buzzer.tone(NOTE_G5, 10000/MUSIC_SPEED);
  delay(12000/MUSIC_SPEED);
  buzzer.tone(NOTE_F5, 6000/MUSIC_SPEED);
  buzzer.tone(NOTE_AS5, 10000/MUSIC_SPEED);
  delay(8000/MUSIC_SPEED);
  buzzer.tone(NOTE_AS5, 6000/MUSIC_SPEED);
  delay(6000/MUSIC_SPEED);
  buzzer.tone(NOTE_A5, 10000/MUSIC_SPEED);
  delay(12000/MUSIC_SPEED);
  buzzer.tone(NOTE_AS5, 6000/MUSIC_SPEED);
  delay(6000/MUSIC_SPEED);
  buzzer.tone(NOTE_A5, 10000/MUSIC_SPEED);
  delay(12000/MUSIC_SPEED);
  buzzer.tone(NOTE_A5, 6000/MUSIC_SPEED);
  delay(6000/MUSIC_SPEED);
  return;
}

void loop_end_tune_1() {
  buzzer.tone(NOTE_G5, 10000/MUSIC_SPEED);
  delay(12000/MUSIC_SPEED);
  buzzer.tone(NOTE_F5, 10000/MUSIC_SPEED);
  delay(12000/MUSIC_SPEED);
  buzzer.tone(NOTE_E5, 10000/MUSIC_SPEED);
  delay(12000/MUSIC_SPEED);
  buzzer.tone(NOTE_F5, 6000/MUSIC_SPEED);
  delay(6000/MUSIC_SPEED);
  buzzer.tone(NOTE_D5, 54000/MUSIC_SPEED);
  delay(54000/MUSIC_SPEED);
  return;
}

void loop_end_tune_2() {
  buzzer.tone(NOTE_G5, 10000/MUSIC_SPEED);
  delay(12000/MUSIC_SPEED);
  buzzer.tone(NOTE_F5, 10000/MUSIC_SPEED);
  delay(12000/MUSIC_SPEED);
  buzzer.tone(NOTE_G5, 10000/MUSIC_SPEED);
  delay(12000/MUSIC_SPEED);
  buzzer.tone(NOTE_AS5, 6000/MUSIC_SPEED);
  delay(6000/MUSIC_SPEED);
  buzzer.tone(NOTE_C6, 54000/MUSIC_SPEED);
  delay(54000/MUSIC_SPEED);
  return;
}

// challenge functions
/**
 * Attempts to solve the color challenge, then calls the appropriate function
 * to move the mBot accordingly. Returns true if a challenge was found and
 * solved, and false otherwise.
 * 
 * @return  True if color was detected and the appropriate action taken, and
 *          false otherwise.
 */
bool solve_color() {
    struct color paper = read_ldr_sensor();
  // solve accordingly; TODO
  if (paper.r > 600 && paper.g > 600 && paper.b > 600) {
    // white
    turn_180();
  } else if (paper.r > 700) {
    // orange
    turn_left_forward_left();
  } else if (paper.r > 600) {
    // red
    turn_left();
  } else if (paper.b > 400) {
    // blue
    turn_right_forward_right();
  } else if (paper.g > 350) {
    // green
    turn_right();
  } else {
    // black
    return false;
  }
  return true;
}

bool solve_sound() {
  int mic_low = analogAvgRead(MIC_LOW);
  int mic_high = analogAvgRead(MIC_HIGH);
  if (mic_low >= MIC_THRESHOLD || mic_high >= MIC_THRESHOLD) {
    if (mic_low > mic_high + MIC_DECIDE) {
      turn_left();
    } else if (mic_high > mic_low + MIC_DECIDE) {
      turn_right();
    } else {
      // both are not louder than the other mic by MIC_DECIDE, therefore, two sounds 
      // have the same amplitude.
      turn_180();
    }
    return true;
  } else {
    return false;
  }
}

void solve_challenge() {
  motor_l.stop();
  motor_r.stop();
  // if (solve_sound()) {
  //   return;
  // } else 
  if (solve_color()) {
    return;
  } else {
    rgbled.setColor(50, 50, 50);
    start_tune();
    while (analogRead(BUTTON) > 10) {
      loop_tune();
      loop_end_tune_1();
      loop_tune();
      loop_end_tune_2();
    }
    rgbled.clear();
    return;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(IR_L, INPUT);
  pinMode(IR_R, INPUT);
  pinMode(LINE, INPUT);
  pinMode(LIGHT, INPUT);
  pinMode(MIC_LOW, INPUT);
  pinMode(MIC_HIGH, INPUT);
  pinMode(MOTOR_L, OUTPUT);
  pinMode(MOTOR_R, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  while (analogRead(BUTTON) > 10) {
    delay(10);
  }
}

void loop() {/*
  // Stop moving and solve challenge if mBot reaches black line.
  if (digitalRead(LINE) == LOW) {
    solve_challenge();
    for (int time = 0; digitalRead(LINE) == LOW && time < TURN_DURATION; time += 10) {
      delay(10);
    }
  // Otherwise, keep moving forward while keeping yourself in the centre using
  // the IR sensors.
  } else {
    move_forward();
  }*/
  Serial.print("Low: ");
  Serial.print(analogRead(MIC_LOW));
  Serial.print(" High: ");
  Serial.println(analogRead(MIC_HIGH));
  delay(500);
  /* DEBUG: Color Test
  struct color test = read_ldr_sensor();
  Serial.print("R");
  Serial.print(test.r);
  Serial.print(" G");
  Serial.print(test.g);
  Serial.print(" B");
  Serial.print(test.b);
  Serial.println("");
  delay(1000);*/
}
