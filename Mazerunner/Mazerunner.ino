#include "pitches.h"   // note definitions for playing MUSIC
#include "mCore.h"     // mcore

// constant definitions
#define ULTRASONIC_TIMEOUT 30000    // timeout for pulseIn (in us)
#define ULTRASONIC_THRESHOLD 600    // threshold below which to turn for double turns (in ms)

#define MIC_THRESHOLD_LOW 300       // threshold value to count as signal for lower frequency
#define MIC_THRESHOLD_HIGH 150      // threshold value to count as signal for higher frequency
#define MIC_DECIDE_LOW 200          // threshold value to decide that low is louder
#define MIC_DECIDE_HIGH 100         // threshold value to decide that high is louder

#define MAX_SPEED 255               // max speed of motors
#define ADJUSTMENT_SPEED 200        // speed to use when adjusting direction
#define SHARP_ADJUSTMENT_SPEED 150  // speed to use when too close to wall
#define TURNING_SPEED 170           // speed to use for single turns (in ms)
#define TURN_DURATION 250           // delay to use for single turns (in ms)
#define TURN_DURATION2 285          // delay to use for second turn of double turn (in ms)
#define TURN_SPEED_MULTIPLIER 0.8   // multiplier on turn speed for all turns

#define LED_DELAY 30                // response time of LDR (in ms)

#define MUSIC_SPEED 1               // default 1 (normal speed)
#define MUSIC_ADJUST 20             // play notes for this duration less than their full duration (in ms)

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

struct color {
  int c;
  int r;
  int g;
  int b;
};

int ir_threshold_l;
int ir_threshold_r;
int ir_sharp_threshold_l;
int ir_sharp_threshold_r; 

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
 * heard. 
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

/**
 * Reads the voltage from the LIGHT pin 4 times under different lighting
 * conditions provided by the mBot's onboard LEDs: No light, red light, 
 * green light and blue light, storing and returning the information as a 
 * struct color. LDR response time is accounted for using delay for a duration
 * specified by LED_DELAY. 
 * 
 * @return result  A struct color containing the 4 values corresponding to
 *                 the colors read. 
 */
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
/**
 * Sets the mBot's motors such that it turns slightly to the left.
 */
void adjust_to_left() {
  motor_r.run(MAX_SPEED);
  motor_l.run(-ADJUSTMENT_SPEED);
  return;
}

/**
 * Sets the mBot's motors such that it turns slightly to the right.
 */
void adjust_to_right() {
  motor_r.run(ADJUSTMENT_SPEED);
  motor_l.run(-MAX_SPEED);
  return;
}

/**
 * Sets the mBot's motors such that it turns sharply to the left.
 */
void adjust_to_sharp_left() {
  motor_r.run(MAX_SPEED);
  motor_l.run(-SHARP_ADJUSTMENT_SPEED);
  return;
}

/**
 * Sets the mBot's motors such that it turns sharply to the right.
 */
void adjust_to_sharp_right() {
  motor_r.run(SHARP_ADJUSTMENT_SPEED);
  motor_l.run(-MAX_SPEED);
  return;
}

/**
 * Turns the mBot 90 degrees to the left.
 */
void turn_left() {
  motor_r.run(TURNING_SPEED * TURN_SPEED_MULTIPLIER);
  motor_l.run(MAX_SPEED * TURN_SPEED_MULTIPLIER);
  delay(TURN_DURATION / TURN_SPEED_MULTIPLIER);
  motor_r.stop();
  motor_l.stop();
  return;
}

/**
 * Turns the mBot 90 degrees to the right.
 */
void turn_right() {
  motor_r.run(-MAX_SPEED * TURN_SPEED_MULTIPLIER);
  motor_l.run(-TURNING_SPEED * TURN_SPEED_MULTIPLIER);
  delay(TURN_DURATION / TURN_SPEED_MULTIPLIER);
  motor_r.stop();
  motor_l.stop();
  return;
}

/**
 * Turns the mBot 180 degrees to the right.
 */
void turn_180() {
  turn_right();
  delay(50);
  turn_right();
  return;
}

/**
 * Moves the mBot forward, reading the IR sensors to determine the ideal
 * direction of movement. 
 */
void move_forward() {    
  if (analogAvgRead(IR_L) < ir_sharp_threshold_l) {
    adjust_to_sharp_right();
  } else if (analogAvgRead(IR_R) < ir_sharp_threshold_r) {
    adjust_to_sharp_left();
  } else if (analogAvgRead(IR_L) < ir_threshold_l) {
    adjust_to_right();
  } else if (analogAvgRead(IR_R) < ir_threshold_r) {
    adjust_to_left();
  } else {
    motor_r.run(MAX_SPEED);
    motor_l.run(-MAX_SPEED);
  }
  return;
}

/**
 * Rotates the mBot 90 degrees to the left, then moves it forward until the
 * ultrasonic sensor returns a value below ULTRASONIC_THRESHOLD, then rotates
 * the mBot 90 degrees to the left again. 
 */
void turn_left_forward_left() {
  turn_left();
  move_forward();

  // while ultrasonic larger than threshold, keep moving forward
  while (read_ultrasonic_sensor() > ULTRASONIC_THRESHOLD) {
    delay(10);
  }

  //turn_left();
  motor_r.stop();
  motor_l.stop();
  delay(100);

  // second turn
  motor_r.run(TURNING_SPEED * TURN_SPEED_MULTIPLIER);
  motor_l.run(MAX_SPEED * TURN_SPEED_MULTIPLIER);
  delay(TURN_DURATION2 / TURN_SPEED_MULTIPLIER);
  motor_r.stop();
  motor_l.stop();
  return;
}

/**
 * Rotates the mBot 90 degrees to the right, then moves it forward until the
 * ultrasonic sensor returns a value below ULTRASONIC_THRESHOLD, then rotates
 * the mBot 90 degrees to the right again. 
 */
void turn_right_forward_right() {
  turn_right();
  move_forward();

  // while ultrasonic larger than threshold, keep moving forward
  while (read_ultrasonic_sensor() > ULTRASONIC_THRESHOLD) {
    delay(10);
  }

  //turn_right();
  motor_r.stop();
  motor_l.stop();
  delay(100);

  // second turn
  motor_r.run(-MAX_SPEED * TURN_SPEED_MULTIPLIER);
  motor_l.run(-TURNING_SPEED * TURN_SPEED_MULTIPLIER);
  delay(TURN_DURATION2 / TURN_SPEED_MULTIPLIER);
  motor_r.stop();
  motor_l.stop();
  return;
}

// victory theme
/**
 * Plays a note for a set duration, then waits for another duration
 * after the note has been played. The playback duration can be 
 * adjusted using MUSIC_ADJUST and MUSIC_SPEED, which reduce the 
 * duration by a constant and a percentage respectively.
 */
void play_note(int note, int duration, int wait) {
  duration /= MUSIC_SPEED;
  wait /= MUSIC_SPEED;
  buzzer.tone(note, duration - MUSIC_ADJUST / MUSIC_SPEED);
  delay(duration + wait);
  return;
}

/**
 * The starting verse of victory fanfare.
 */
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

/**
 * The first half of the looping portion of victory fanfare.
 */
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

/**
 * The second half of the looping portion of victory fanfare.
 */
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

/**
 * Plays the mario coin sound effect.
 */
void challenge_complete() {
  play_note(NOTE_B6, 50, 0);
  buzzer.tone(NOTE_E7, 350);
  return;
}

/**
 * Converts a voltage to an equivalent value directly proportional to 
 * light intensity hitting the LDR. Note that the formula has not been
 * experimentally proven, and assumes that the relationship between the
 * voltage and intensity resembles a curve similar to that of a charging
 * capacitor, with intensity replacing time. 
 * 
 * @param[in] value  The voltage read from the LIGHT pin
 * @return           A value directly proportional to light intensity hitting
 *                   the LDR. 
 */
float find_intensity(int value) {
  return -log(1 - (float) value / 1023);
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
  if (red_green > 1.6) {
    if ((find_intensity(paper.g) - find_intensity(paper.c)) > 0.2) {
      // orange 0.21
      rgbled.setColor(128, 64, 0);
      turn_left_forward_left();
    } else {
      // red 0.19
      rgbled.setColor(192, 0, 0);
      turn_left();
    }
  } else if (red_green > 1.15) {
    if (paper.r > 400) {
      // white
      rgbled.setColor(64, 64, 64);
      turn_180();
    } else {
      // black
      return false;
    }
  } else {
    if (paper.b > paper.r) {
      // blue
      rgbled.setColor(0, 0, 192);
      turn_right_forward_right();
    } else {
      // green
      rgbled.setColor(0, 192, 0);
      turn_right();
    }
  }
  return true;
}

/**
 * Attempts to solve the sound challenge, then calls the appropriate function
 * to move the mBot accordingly. Returns true if a challenge was found and
 * solved, and false otherwise.
 * 
 * @return  True if instruction sounds were detected and the appropriate action taken, and
 *          false otherwise.
 */
bool solve_sound() {
  int low_freq = analogAvgRead(MIC_LOW);
  int high_freq = analogAvgRead(MIC_HIGH);
  int temp_high;
  int temp_low;
  for (int i = 1; i < 10; i += 1) {
    temp_high = analogAvgRead(MIC_HIGH);
    temp_low = analogAvgRead(MIC_LOW);
    high_freq = temp_high < high_freq ? temp_high : high_freq;
    low_freq = temp_low < low_freq ? temp_low : low_freq;
    delay(10);
  }
  if (high_freq < MIC_THRESHOLD_HIGH && low_freq < MIC_THRESHOLD_LOW) {
    return false;
  }
  if (low_freq > high_freq + MIC_DECIDE_LOW) {
    turn_left();
  } else if (high_freq > low_freq + MIC_DECIDE_HIGH) {
    turn_right();
  } else {
    // both are not louder than the other mic by the respective differences, 
    // therefore, two sounds have the same amplitude.
    turn_180();
  }
  return true;
}

/**
 * Continuously plays the celebratory tune.
 */
void finish_race() {
  start_tune();
  while (1) {
    loop_tune_1();
    loop_tune_2();
  }
  return;
}

/**
 * Attempts to solve a challenge by trying color and then sound. Plays the
 * celebratory tune if no actions were taken, which means the end of the race
 * has been reached.
 */
void solve_challenge() {
  motor_l.stop();
  motor_r.stop();
  rgbled.clear();
  delay(150);
  motor_l.run(ADJUSTMENT_SPEED);
  motor_r.run(-ADJUSTMENT_SPEED);
  delay(100);
  motor_l.stop();
  motor_r.stop();
  delay(100);
  if (!solve_color() && !solve_sound()) {
    finish_race();
  }
  challenge_complete();
  return;
}

/**
 * Reads the analog output of a pin 32 times, and returns the average value
 * rounded down to the nearest integer. Intended for calibrating the IR sensor.
 *
 * @param[in] pin  The pin to read
 * @return         The average value of 32 readings from the pin
 */
int calibrate_ir(int pin) {
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
  ir_threshold_l = calibrate_ir(IR_L) * 0.95;
  ir_threshold_r = calibrate_ir(IR_R) * 0.95;
  ir_sharp_threshold_l = ir_threshold_l * 0.8;
  ir_sharp_threshold_r = ir_threshold_r * 0.8;
}

void loop() {
  // Stop moving and solve challenge if mBot reaches black line.
  if (digitalRead(LINE) == LOW) {
    // motor_l.run(ADJUSTMENT_SPEED);
    // motor_r.run(-ADJUSTMENT_SPEED);
    // delay(180);
    // motor_l.stop();
    // motor_r.stop();
    // for (int i = 0; i < 10; i += 1) {
    //   delay(20);
    //   if (digitalRead(LINE) != LOW) {
    //     delay(100);
    //     return;
    //   }
    // }
    solve_challenge();
    int wait = 0;
    while (digitalRead(LINE) == LOW && wait < 200) {
      wait += 10;
      delay(10);
    }
    // Otherwise, keep moving forward while keeping yourself in the centre using
    // the IR sensors.
  } else {
    move_forward();
  }

  /* DEBUG: Mic Test
     Serial.print("Low: ");
     Serial.print(analogAvgRead(MIC_LOW));
     Serial.print(" High: ");
     Serial.println(analogAvgRead(MIC_HIGH));
     delay(500);
   */

  /* DEBUG: Color Test
     struct color test = read_ldr_sensor();
     Serial.print(find_intensity(test.g) - find_intensity(test.c), 7);
     Serial.print(find_intensity(test.g) - find_intensity(test.c) > 0.2 ? "Orange" : "Red");
     Serial.print("\t");
     Serial.print(test.c);
     Serial.print("\t");
     Serial.print(test.r);
     Serial.print("\t");
     Serial.print(test.g);
     Serial.print("\t");
     Serial.print(test.b);
     Serial.println("");
     delay(1000);
   */

  /* DEBUG: Ultrasonic Sensors
     Serial.println(read_ultrasonic_sensor());
     delay(1000);
   */
}
