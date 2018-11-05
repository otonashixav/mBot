#include <Arduino.h>   // should be included automatically when compiled; includes stdbool
#include "pitches.h"   // note definitions for playing MUSIC
#include "mCore.h"     // mcore

// constant definitions
#define ULTRASONIC_TIMEOUT 30000
#define ULTRASONIC_THRESHOLD
#define INFRARED_THRESHOLD
#define MIC_THRESHOLD
#define MAX_SPEED 255
#define ADJUSTMENT_SPEED 200

// pin definitions
// Port 1 contains 2 digital pins 11 and 12
// Port 2 contains 2 digital pins 9 and 10
// Port 3 contains 2 analog pins A2 and A3
// Port 4 contains 2 analog pins A0 and A1
#define ULTRASONIC 12  // ultrasonic sensor
#define LINR_L 9       // left line sensor  
#define LINE_R 10      // right line sensor
#define LIGHT A6       // light sensor
#define LED 13         // top led
#define IR_L A0        // left infrared sensor
#define IR_R A2        // right infrared sensor
#define MIC_LOW A1     // low pass mic output
#define MIC_HIGH A3    // high pass mic output
#define MOTOR_L M1     // left motor
#define MOTOR_R M2     // right motor
#define BUZZER 8       // buzzer

// assign classes
MeDCMotor motor_l(MOTOR_L);
MeDCMotor motor_r(MOTOR_R);
MeRGBLed rgbled(LED);
MeBuzzer buzzer(BUZZER);

// motor functions
void turn_left() {

}

void turn_right() {

}

void turn_180() {

}

void move_forward() {

}

// victory theme


void play_theme() {

}

// sensor functions

/**
 * Sends an ultrasonic pulse and waits for the echo. The timeout is defined
 * by ULTRASONIC_TIMEOUT. Returns the time in microseconds before the echo is
 * heard. Consider replacing with boolean function for "close enough to wall"
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

long read_ldr_sensor() {
    analogRead(LIGHT);
}

// challenge functions

int solve_color() {

}

int solve_sound() {

}

int solve_challenge() {

}

void setup() {
    Serial.begin(9600);
}

void loop() {
    rgbled.setColor(255, 255, 255);
    delay(1000);
    Serial.println(read_ldr_sensor());
    rgbled.setColor(255, 0, 0);
    delay(1000);
    Serial.println(read_ldr_sensor());
    rgbled.setColor(0, 255, 0);
    delay(1000);
    Serial.println(read_ldr_sensor());
    rgbled.setColor(0, 0, 255);
    delay(1000);
    Serial.println(read_ldr_sensor());
    rgbled.clear();
    delay(1000);
    Serial.println(read_ldr_sensor());
}