#include "pitches.h"   // note definitions for playing MUSIC
#include "mCore.h"     // mcore

struct color {
  int c;
  int r;
  int g;
  int b;
};

// constant definitions
#define ULTRASONIC_TIMEOUT 30000         // timeout for pulseIn
#define ULTRASONIC_THRESHOLD 600         // "close enough to wall to turn"

#define INFRARED_THRESHOLD_L 650         // "close enough to wall to adjust to left" PREV: 550
#define INFRARED_THRESHOLD_R 650         // "close enough to wall to adjust to right" PREV: 550
#define SHARP_INFRARED_THRESHOLD_L 400   // "too close to wall to adjust to left"
#define SHARP_INFRARED_THRESHOLD_R 400   // "too close to wall to adjust to right"

#define MIC_THRESHOLD 50                 // "loud enough to be considered not noise"
#define MIC_DECIDE_LOW 200               // threshold value to decide that low is louder
#define MIC_DECIDE_HIGH 100              // threshold value to decide that high is louder

#define MAX_SPEED 255                    // max speed of motors
#define ADJUSTMENT_SPEED 200             // speed to use when adjusting direction
#define SHARP_ADJUSTMENT_SPEED 150       // speed to use when too close to wall
#define ADJUSTMENT_DELAY 1000            // time to stay at adjustment speed
#define TURNING_SPEED 200                //
#define TURN_DURATION 255                //
#define TURN_DURATION2 400                //

#define LED_DELAY 30                     // response time of LDR

#define FORWARD_INTERVAL 1000            //

#define MUSIC_SPEED 1                    // default 1 (normal speed)
#define MUSIC_ADJUST 20                  // play notes for this duration less than their full duration (in ms)

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
#define MOTOR_L M2     // left motor
#define MOTOR_R M1     // right motor
#define BUZZER 8       // buzzer
#define BUTTON A7       // button

// assign classes
MeDCMotor motor_l(MOTOR_L);
MeDCMotor motor_r(MOTOR_R);
MeRGBLed rgbled(LED);
MeBuzzer buzzer(BUZZER);

int ir_tshold_l;
int ir_tshold_r;
int ir_shrp_tshold_l;
int ir_shrp_tshold_r; 

// sensor functions

/**
 * Reads the analog output of a pin 5 times, and returns the average value
 * rounded down to the nearest integer. 
 * 
 * @param[in] pin  The pin to read
 * @return         The average value of 5 readings from the pin
 */
int analogAvgRead(int pin) {
  int sum = 0;
  for (int i = 0; i < 10; i += 1) {
    sum += analogRead(pin);
  }
  return sum / 10; // rounds down but negligible for our purposes 
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
  result.c = analogAvgRead(LIGHT);
  rgbled.setColor(255, 0, 0);
  delay(LED_DELAY);
  result.r = analogAvgRead(LIGHT);
  rgbled.setColor(0, 255, 0);
  delay(LED_DELAY);
  result.g = analogAvgRead(LIGHT);
  rgbled.setColor(0, 0, 255);
  delay(LED_DELAY);
  result.b = analogAvgRead(LIGHT);
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


void adjust_to_sharp_left() {
  motor_r.run(MAX_SPEED);
  motor_l.run(-SHARP_ADJUSTMENT_SPEED);
  return;
}

void adjust_to_sharp_right() {
  motor_r.run(SHARP_ADJUSTMENT_SPEED);
  motor_l.run(-MAX_SPEED);
  return;
}

void turn_left() {
  motor_r.run(TURNING_SPEED);
  motor_l.run(MAX_SPEED);
  delay(TURN_DURATION);
  motor_r.stop();
  motor_l.stop();
  return;
}

void turn_right() {
  motor_r.run(-MAX_SPEED);
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
  if (analogAvgRead(IR_L) < ir_shrp_tshold_l) {
    adjust_to_sharp_right();
  } else if (analogAvgRead(IR_L) < ir_tshold_l) {
    adjust_to_right();
  } else if (analogAvgRead(IR_R) < ir_shrp_tshold_r) {
    adjust_to_sharp_left();
  } else if (analogAvgRead(IR_R) < ir_tshold_r) {
    adjust_to_left();
  } else {
    motor_r.run(MAX_SPEED);
    motor_l.run(-MAX_SPEED);
  }
  return;
}

// TODO: Review below code. //consider put move_forward() function into the while loop?
void turn_left_forward_left() {
  turn_left();
  move_forward();

  // while ultrasonic larger than threshold, keep moving forward
  while (read_ultrasonic_sensor() > ULTRASONIC_THRESHOLD) {
    delay(10);
  }

  //turn_left();

  motor_r.run(TURNING_SPEED);
  motor_l.run(MAX_SPEED);
  delay(TURN_DURATION2);
  motor_r.stop();
  motor_l.stop();
  return;
}


void turn_right_forward_right() {
  turn_right();
  move_forward();

  // while ultrasonic larger than threshold, keep moving forward
  while (read_ultrasonic_sensor() > ULTRASONIC_THRESHOLD) {
    delay(10);
  }

  //turn_right();

  motor_r.run(-MAX_SPEED);
  motor_l.run(-TURNING_SPEED);
  delay(TURN_DURATION2);
  motor_r.stop();
  motor_l.stop();
  return;
}

// victory theme

void play_note(int note, int duration, int wait) {
  duration /= MUSIC_SPEED;
  wait /= MUSIC_SPEED;
  buzzer.tone(note, duration - MUSIC_ADJUST / MUSIC_SPEED);
  delay(duration + wait);
  return;
}

void start_tune() {
  play_note(NOTE_C6, 80, 80);
  play_note(NOTE_C6, 80, 80);
  play_note(NOTE_C6, 80, 80);
  play_note(NOTE_C6, 480, 0);
  play_note(NOTE_GS5, 480, 0);
  play_note(NOTE_AS5, 480, 0);
  play_note(NOTE_C6, 180, 180);
  play_note(NOTE_AS5, 120, 0);
  play_note(NOTE_C6, 1440, 0);
  return;
}

void loop_tune_1() {
  play_note(NOTE_G5, 480, 0);
  play_note(NOTE_F5, 480, 0);
  play_note(NOTE_G5, 480, 0);
  play_note(NOTE_F5, 120, 120);
  play_note(NOTE_AS5, 480, 0);
  play_note(NOTE_AS5, 120, 120);
  play_note(NOTE_A5, 480, 0);
  play_note(NOTE_AS5, 120, 120);
  play_note(NOTE_A5, 480, 0);
  play_note(NOTE_A5, 120, 120);
  play_note(NOTE_G5, 480, 0);
  play_note(NOTE_F5, 480, 0);
  play_note(NOTE_E5, 480, 0);
  play_note(NOTE_F5, 120, 120);
  play_note(NOTE_D5, 2160, 0);
  return;
}

void loop_tune_2() {
  play_note(NOTE_G5, 480, 0);
  play_note(NOTE_F5, 480, 0);
  play_note(NOTE_G5, 480, 0);
  play_note(NOTE_F5, 120, 120);
  play_note(NOTE_AS5, 480, 0);
  play_note(NOTE_AS5, 120, 120);
  play_note(NOTE_A5, 480, 0);
  play_note(NOTE_AS5, 120, 120);
  play_note(NOTE_A5, 480, 0);
  play_note(NOTE_A5, 120, 120);
  play_note(NOTE_G5, 480, 0);
  play_note(NOTE_F5, 480, 0);
  play_note(NOTE_G5, 480, 0);
  play_note(NOTE_AS5, 120, 120);
  play_note(NOTE_C6, 2160, 0);
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
  float red_green = (float) paper.r / paper.g;
  if (red_green > 2.05) { //2.1
    // red
    turn_left();
  } else if (red_green > 1.6) {
    // orange
    turn_left_forward_left();
  } else if (red_green > 1.15) {
    if (paper.r > 400) {
      // white
      turn_180();
    } else {
      // black
      return false;
    }
  } else {
    if (paper.b > paper.r) {
      // blue
      turn_right_forward_right();
    } else {
      // green
      turn_left_forward_left();
    }
  }
  return true;
}

/**
 * Attempts to solve the sound challenge, then calls the appropriate function
 * to move the mBot accordingly. Returns true if a challenge was found and
 * solved, and false otherwise.
 * 
 * @return  true if instruction sounds were detected and the appropriate action taken, and
 *          false otherwise.
 */
bool solve_sound() {
  int mic_low = analogAvgRead(MIC_LOW);
  int mic_high = analogAvgRead(MIC_HIGH);
  int temp_high;
  int temp_low;
  for (int i = 1; i < 10; i += 1) {
    temp_high = analogAvgRead(MIC_HIGH);
    temp_low = analogAvgRead(MIC_LOW);
    mic_high = temp_high < mic_high ? temp_high : mic_high;
    mic_low = temp_low < mic_low ? temp_low : mic_low;
    delay(10);
  }
  if (mic_high >= MIC_THRESHOLD) {
    if (mic_low > mic_high + MIC_DECIDE_LOW) {
      turn_left();
    } else if (mic_high > mic_low + MIC_DECIDE_HIGH) {
      turn_right();
    } else {
      // both are not louder than the other mic by MIC_DECIDE, 
      // therefore, two sounds have the same amplitude.
      turn_180();
    }
    return true;
  } else {
    return false;
  }
}

void finish_race() {
  rgbled.setColor(100, 100, 100);
  start_tune();
  while (analogRead(BUTTON) > 10) {
    loop_tune_1();
    loop_tune_2();
  }
  rgbled.clear();
  return;
}

/**
 * Attempts to decide which challenge it needs to solve, then calls the appropriate function
 * to solve the challenge. Play the celebratory tune at the end of the maze
 */
void solve_challenge() {
  if (!solve_color() && !solve_sound()) {
    finish_race();
  }
  return;
}

int calibrateIR(int pin) {
  int sum = 0;
  for (int i = 0; i < 32; i += 1) {
    sum += analogRead(pin);
    delay(10);
  }
  return sum / 32; // rounds down but negligible for our purposes 
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
  ir_tshold_l = calibrateIR(IR_L) /10 *9;
  ir_tshold_r = calibrateIR(IR_R) /10 *9;
  ir_shrp_tshold_l = ir_tshold_l /10 *7
    ir_shrp_tshold_r = ir_tshold_r /10 *7;

  Serial.println(ir_tshold_l);
  Serial.println(ir_tshold_r);
  Serial.println(ir_shrp_tshold_l);
  Serial.println(ir_shrp_tshold_r);
}

void loop() {/*
  // Stop moving and solve challenge if mBot reaches black line.
  if (digitalRead(LINE) == LOW) {
    motor_l.stop();
    motor_r.stop();
    for (int i = 0; i < 10; i += 1) {
      delay(20);
      if (digitalRead(LINE) != LOW) {
        return;
      }
    }
    solve_challenge();
    // Otherwise, keep moving forward while keeping yourself in the centre using
    // the IR sensors.
  } else {
    move_forward();
  }
<<<<<<< HEAD
     Serial.print("Low: ");
     Serial.print(analogAvgRead(MIC_LOW));
     Serial.print(" High: ");
     Serial.println(analogAvgRead(MIC_HIGH));
     delay(500);
  /* */DEBUG: Color Test
=======

  /* DEBUG: Mic Test
     Serial.print("Low: ");
     Serial.print(analogAvgRead(MIC_LOW));
     Serial.print(" High: ");
     Serial.prin8tln(analogAvgRead(MIC_HIGH));
     delay(500);*/

  /* DEBUG: Color Test
>>>>>>> a9d7d22919128d95b36a0b9c6c380a446cf25252
     struct color test = read_ldr_sensor();
     Serial.print("R");
     Serial.print(test.r);
     Serial.print(" G");
     Serial.print(test.g);
     Serial.print(" B");
     Serial.print(test.b);
     Serial.println("");
     delay(1000);

  /* DEBUG: Ultrasonic Sensors
     Serial.println(read_ultrasonic_sensor());
     delay(1000);*/




}
