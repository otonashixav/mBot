#include <Arduino.h> // should be included automatically when compiled; includes stdbool
#include "pitches.h" // note definitions for playing MUSIC

// time definitions
#define ULTRASONIC_TIMEOUT 30000

// pin definitions
#define ULTRASONIC 12  // ultrasonic sensor
#define LINE           // line sensor
#define LIGHT          // light sensor
#define LED            // top led
#define IR_L           // left infrared sensor
#define IR_R           // right infrared sensor
#define MIC_LOW        // low pass mic output
#define MIC_HIGH       // high pass mic output
#define MOTOR_L        // left motor
#define MOTOR_R        // right motor

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

bool read_line_sensor() {
    
}

// challenge functions

int solve_color() {

}

int solve_sound() {

}

int solve_challenge() {

}

void setup() {

}

void loop() {
    if (read_line_sensor() == HIGH) {
        solve_challenge();
    } else {
        // read ir sensors and determine if correction necessary, else straight

    }
}