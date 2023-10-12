/*
 *
 * EE30186 Integrated Engineering
 *
 * DECEMBER 2022
 *
 */

/****************************** DEPENDENCIES ******************************/
#include <cstdio>
#include "PinNamesTypes.h"
#include "PwmOut.h"
#include "ThisThread.h"
#include "mbed.h"
#include <queue>
#include "TextLCD.h"
#include <chrono>
 
/*************************** OBJECTS & DEFINITIONS ************************/
// Create input objects
DigitalIn rotaryA(PA_1);
DigitalIn rotaryB(PA_4);

// Create output objects
DigitalOut LEDGreen(PB_7);
DigitalOut LEDRed(PA_15);
PwmOut pwm(PB_0);

// Define tachometer signal, button and LCD pins
InterruptIn taco_signal(PA_0);
InterruptIn pushButton(PC_13);
TextLCD lcd(PB_15, PB_14, PB_10, PA_8, PB_2, PB_1); // rs, e, d4-d7

/****************************** STRUCTURES ********************************/
struct states {
    int current_state;
    int prev_state; 
};

struct flags {
    int speedFlag = 0;
    int calcFlag = 0;
    int targetFlag = 0;
};

struct fan {
    float RPM;
    int pulse = 0;
    float maxRPM = 1950;
    Ticker currentRPM;
};

struct control {
    int TargetRPM;
    float openLoopDutyCycle;
    float closedLoopDutyCycle;
    float pulseStretchDuration = 1;
    float Kp = 1.8; // PID proportional constant
    float Ki = 1.1; // PID integral constant
    float Kd = 0.05; // PID derivative constant
    Timer PIDTimer;
};

/******************************* VARIABLES ********************************/
struct states s;
struct flags f;
struct fan fan;
struct control c;
float counter = 50;

/******************************* FUNCTIONS ********************************/
// PID function for closed-loop control
void PID() {
    // Reset time parameters for PID calculation 
    c.PIDTimer.stop();
    float deltaT = c.PIDTimer.read();
    c.PIDTimer.reset();

    // Initialise variables and calculate error
    static int previousError;
    float error = c.TargetRPM - fan.RPM;
    static int integral =+ (error * deltaT);
    float derivative = (error - previousError) / deltaT;
    float closedLoopSpeed = c.TargetRPM + (c.Kp * error) + (c.Ki * integral) + (c.Kd * derivative);

    // Add pulse stretching for additional accuracy
    static float previousClosedLoopSpeed = closedLoopSpeed;
    if (closedLoopSpeed > previousClosedLoopSpeed) {
        c.closedLoopDutyCycle += c.pulseStretchDuration;
    } else {
        c.closedLoopDutyCycle -= c.pulseStretchDuration;
    }

    previousClosedLoopSpeed = closedLoopSpeed;
    c.closedLoopDutyCycle = counter / 100;
    previousError = error;
}

// Function to clear LCD screen when switching between control systems
void lcdClear(){
    if (pushButton.read() == 0){
        lcd.cls();   
    }
}

// Interrupt function to change LED colour on each press of the push button
void buttonPress(){
    LEDGreen = !LEDGreen;
    LEDRed = !LEDRed;
}

// Encoder function to read user input
float encoder(){
    s.current_state = rotaryA;
    if (s.prev_state != s.current_state){
        if (rotaryB != s.current_state){
            counter++;
        }
        else {
            counter--;
        }
        if (counter < 0){
            counter = 0;
        }
        if (counter > 100){
            counter = 100;
        }
    s.prev_state = s.current_state;
    }
    return counter;
}

// Switch case function for various states of the LCD screen in open loop
void openLoopLCD(){
    if (counter == 100 && f.speedFlag == 0){
        lcd.cls();
        f.speedFlag++;
    } else if (counter == 100 && f.speedFlag == 1){
        lcd.printf("Speed: %.0f%%\n", counter);
        lcd.printf("Open loop\n");
    } else if (counter > 9 && counter < 100 && f.speedFlag == 1){
        f.speedFlag = 0;
        lcd.cls();
    } else if (counter > 9 && counter < 100 && f.speedFlag == 0){
        lcd.printf("Speed: %.0f%%\n", counter);
        lcd.printf("Open loop\n");
    } else if (counter < 10 && f.speedFlag == 0){
        lcd.cls();
        f.speedFlag++;
    } else if (counter < 10 && f.speedFlag == 1) {
        lcd.printf("Speed: %.0f%%\n", counter);
        lcd.printf("Open loop\n");
    }
}

// Switch case function for various states of the LCD screen in closed loop
void closedLoopLCD(){
    if (fan.RPM < 1000 && f.speedFlag == 0){
        lcd.cls();
        f.speedFlag++;
    } else if (c.TargetRPM < 1000 && f.targetFlag == 0){
        lcd.cls();
        f.targetFlag++;           
    } else if (fan.RPM < 1000 && f.speedFlag == 1 && f.targetFlag == 0){
        lcd.printf("S:%.0fRPM  Close\n", fan.RPM);
        lcd.printf("T:%dRPM  Loop\n", c.TargetRPM);
    } else if (fan.RPM < 100 && fan.RPM > 9 && f.speedFlag == 1 && f.targetFlag == 1){
        lcd.printf("S:%.0fRPM   Close\n", fan.RPM);
        lcd.printf("T:%dRPM   Loop\n", c.TargetRPM); 
    } else if (fan.RPM < 10 && f.speedFlag == 1 && f.targetFlag == 1){
        lcd.printf("S:%.0fRPM    Close\n", fan.RPM);
        lcd.printf("T:%dRPM   Loop\n", c.TargetRPM);                 
    } else if (c.TargetRPM < 1000 && f.targetFlag == 1 && f.speedFlag == 1){
        lcd.printf("S:%.0fRPM  Close\n", fan.RPM);
        lcd.printf("T:%dRPM   Loop\n", c.TargetRPM);       
    } else if (fan.RPM > 1000 && f.speedFlag == 1){
        f.speedFlag = 0;
        lcd.cls();
    } else if (c.TargetRPM > 1000 && f.targetFlag == 1){
        f.targetFlag = 0;
        lcd.cls();        
    } else if (fan.RPM > 1000 && f.speedFlag == 0 && f.targetFlag == 0){
        lcd.printf("S:%.0fRPM Close\n", fan.RPM);
        lcd.printf("T:%dRPM  Loop\n", c.TargetRPM);
    } else if (c.TargetRPM > 1000 && f.targetFlag == 0 && f.speedFlag == 1){
        lcd.printf("S:%.0fRPM Close\n", fan.RPM);
        lcd.printf("T:%dRPM  Loop\n", c.TargetRPM);
    }
}

// Function to increment pulse from tachometer
void pulseIncrement(){
    fan.pulse++;
}

// Function to signal that the main loop should calulate the fan speed 
void calculateSpeed() {
    f.calcFlag = 1;
}

/******************************* MAIN LOOP ********************************/
int main(){
    // Set the initial 0.2 second time period
    pwm.period(0.2f); 

    // Set up interrupts for the tachometer signal, button and ticker
    taco_signal.mode(PullUp);
    taco_signal.fall(&pulseIncrement);
    pushButton.rise(&buttonPress);
    fan.currentRPM.attach(&calculateSpeed, 1000ms);

    // Set intial state of 
    LEDGreen = 1;
    LEDRed = 0;

    // Loop indefinitely
    while (true) {
        // Check if push button is being pressed, to clear LCD screen
        lcdClear();

        // Check for changes in the state of the encoder
        encoder();

        // If the calculate flag is triggered, calculate speed and reset flags
        if (f.calcFlag == 1) {
            fan.RPM = ((((float)fan.pulse/2) * float(60))/fan.maxRPM)*1000;
            fan.pulse = 0;
            f.calcFlag = 0;
        }
        // Read state of LED, to decide on which control system
        // Enter open loop control 
        if (LEDGreen == 1){
            // Calculate duty cycle from the encoder value
            c.openLoopDutyCycle = encoder() / 100;
            // Write the duty cycle to the PWM output
            pwm.write(c.openLoopDutyCycle); 
            // Output information to LCD screen
            openLoopLCD();

        // Enter closed loop control     
        } else {
            // Set up target RPM from encoder
            c.TargetRPM = (counter * 13) + 450;
            // Start timer for PID calculations
            c.PIDTimer.start();
            // Call PID function to constantly update error
            PID();  
            // Write the duty cycle to the PWM output          
            pwm.write(c.closedLoopDutyCycle);
            // Output information to LCD screen
            closedLoopLCD();
            }
    }
}
