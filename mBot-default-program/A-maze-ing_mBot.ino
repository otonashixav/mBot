#include <Arduino.h>   // should be included automatically when compiled; includes stdbool
#include "pitches.h"   // note definitions for playing MUSIC
#include "mCore.h"     // mcore

// constant definitions
#define ULTRASONIC_TIMEOUT 30000  // timeout for pulseIn
#define ULTRASONIC_THRESHOLD      // "close enough to wall to turn"
#define INFRARED_THRESHOLD_L 600  // "close enough to wall to adjust to left"
#define INFRARED_THRESHOLD_R 400  // "close enough to wall to adjust to right"
#define MIC_THRESHOLD 200         // "loud enough to be considered not noise"
#define MIC_DECIDE 200            // threshold value to decide mic loudness
#define MAX_SPEED 255             // max speed of motors
#define ADJUSTMENT_SPEED 200      // speed to use when adjusting direction
#define ADJUSTMENT_DELAY 1000     // time to stay at adjustment speed
#define LED_DELAY 40              // response time of LDR
#define INFRARED_NO_WALL 700      // no wall in range if analogread returns larger than this
#define IDEAL_DIVISOR 5           // 
#define SMOOTHING 5               // 
#define TURNING_SPEED 255         //
#define TURN_DURATION 1000        //
#define FORWARD_INTERVAL 1000     //

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
#define MIC_LOW A0     // low pass mic output
#define MIC_HIGH A3    // high pass mic output
#define MOTOR_L M1     // left motor
#define MOTOR_R M2     // right motor
#define BUZZER 8       // buzzer

// assign classes
MeDCMotor motor_l(6, 7);
MeDCMotor motor_r(5, 4);
MeRGBLed rgbled(LED);
MeBuzzer buzzer(BUZZER);

// motor functions
void turn_left() {
  motor_r.run(-TURNING_SPEED);
  motor_l.run(-TURNING_SPEED);
  delay(TURN_DURATION);
  return;
}

void turn_right() {
  motor_r.run(TURNING_SPEED);
  motor_l.run(TURNING_SPEED);
  delay(TURN_DURATION);
  return;
}

void turn_180() {
  turn_left();
  turn_left();
  return;
}

void move_forward() {
  motor_r.run(MAX_SPEED);
  motor_l.run(-MAX_SPEED);
  return;
}

void turn_left_forward_left() {
  turn_left();
  // while ultrasonic larger than value, do nothing
  turn_left();
  return;
}

void turn_right_forward_right() {
  turn_right();
  // while ultrasonic larger than value, do nothing
  turn_right();
  return;
}

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

/*
void forward_corrections() {
  int left_ir = analogAvgRead(IR_L);
  int right_ir = analogAvgRead(IR_R);
  static int average_reading = INFRARED_THRESHOLD;
  static int average_gradient = 0;
  // Use the lowest reading
  bool use_left = left_ir < right_ir;
  int reading = use_left ? left_ir : right_ir;
  if (reading < INFRARED_NO_WALL) {
    // Update average readings if it senses a wall
    average_reading += (reading - average_reading) / SMOOTHING;
    average_gradient += (reading - prev_reading - average_gradient) / SMOOTHING;
  }
  if (average_reading < INFRARED_THRESHOLD) {
    // Turn if below threshold
    // Makes the gradient dependent on how close to the wall it is
    int ideal_gradient = (INFRARED_THRESHOLD - average_reading) / IDEAL_DIVISOR;

    int turn_sharpness = ideal_gradient - average_gradient * (1000 / INFRARED_PERIOD);
    // Use constant sharpness?
    //int turn_sharpness = ideal_gradient > average_gradient ? 55 : -55;

    // +ve gradient/sharpness: away from wall
    // Closer to left side: +ve gradient to right
    // Closer to right side: +ve gradient to left
    if (use_left) {
      // Standardise +ve sharpness to left
      turn_sharpness = -turn_sharpness;
    }
    if (turn_sharpness > 0) {
      motor_l.run(MAX_SPEED - turn_sharpness);
    } else {
      motor_r.run(MAX_SPEED - turn_sharpness);
    }
  } else {
    move_forward();
  }
}
*/
// victory theme

void play_theme() {

}

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

struct cRGB read_ldr_sensor() {
  struct cRGB result;
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
    struct cRGB paper = read_ldr_sensor();
  // solve accordingly; TODO
  if (paper.r > 800 && paper.g > 800 && paper.b > 800) {
    // white
    turn_180();
    return true;
  } else if (paper.r > 800 && paper.g > 800) {
    // orange
    turn_left_forward_left();
    return true;
  } else if (paper.r > 800) {
    // red
    turn_left();
    return true;
  } else if (paper.g > 800) {
    // green
    turn_right();
    return true;
  } else if (paper.b > 800) {
    // blue
    turn_right_forward_right();
    return true;
  } else {
    // black
    return false;
  }
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
      //both are not louder than the other mic by MIC_DECIDE, therefore, two sounds 
      //have the same amplitude.
      turn_180();
    }
    return true;
  } else {
    return false;
  }
}

void solve_challenge() {
  if (solve_sound()) {
    return;
  } else if (solve_color()) {
    return;
  } else {
    play_theme();
    // probably loop here until the button is pressed which should return
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
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  if (digitalRead(LINE) == LOW) {
    solve_challenge();
  } else {
    while (analogAvgRead(IR_L) < INFRARED_THRESHOLD_L) {
      adjust_to_right();
    }
    while (analogAvgRead(IR_R) < INFRARED_THRESHOLD_R) {
      adjust_to_left();
    }
    move_forward();
  }
}
