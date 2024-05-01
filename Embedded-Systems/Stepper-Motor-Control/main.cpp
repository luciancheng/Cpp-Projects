#include "mbed.h"
#include "LCD_DISCO_F429ZI.h"
#include <chrono>
#include <cstdlib>
#include <bitset>
#include <inttypes.h>

/* 

Angular resolution (full step) = 360 deg / 48 = 7.5 deg
Angular resolution (half step) = 360 deg / 96 = 3.75 deg

======================================================
Student 1: Lucian Cheng - 400398983
Time Period: 83 - 33 = 50s

Calculation:
--
Full step: 50/48 = 1.04s per step or 1040ms
Falf step: 50/(48 * 2) = 50/96 = 0.521s per step or 521ms

======================================================
Student 2: Melody Yang - 400307007
Time Period: 07 + 33 = 40s

Calculation: 
--
Full step: 40/48 = 0.833s per step or 833ms
Falf step: 40/(48 * 2) = 40/96 = 0.417s per step or 417ms

*/

#define LCD_WIDTH 240
#define LCD_HEIGHT 320
#define DEBOUNCE_PATTERN 0x7FFF

LCD_DISCO_F429ZI LCD;

InterruptIn userButton(BUTTON1);
DigitalOut s_red(PG_1);
DigitalOut s_gray(PE_8);
DigitalOut s_yellow(PE_10);
DigitalOut s_black(PE_12);

// external buttons
InterruptIn b1(PA_6, PullUp);
InterruptIn b2(PA_7, PullUp);
InterruptIn b3(PA_5, PullUp);
InterruptIn b4(PA_3, PullUp);

// shift registers
uint16_t shiftRegButton1;
uint16_t shiftRegButton2;
uint16_t shiftRegButton3;
uint16_t shiftRegButton4;

DigitalOut led_1(PG_14);

Ticker stepper;

// global variables
int step_full = 0;
int step_half = 0;
int num_step_full = 4;
int num_step_half = 8;

int curStudent = 1;
int running = 0;

bool fullstep = true;
bool cw = true;
int time_interval;

// function prototypes
void updateLCD(void);
void switchStudent(void);
int calcTime(int curStudent, int fullstep);


// Full step steps
void Step1_full() {
    s_red = 1;
    s_gray = 0;
    s_yellow = 1;
    s_black = 0;
}

void Step2_full() {
    s_red = 1;
    s_gray = 0;
    s_yellow = 0;
    s_black = 1;
}

void Step3_full() {
    s_red = 0;
    s_gray = 1;
    s_yellow = 0;
    s_black = 1;
}

void Step4_full() {
    s_red = 0;
    s_gray = 1;
    s_yellow = 1;
    s_black = 0;
}

void (*state_table_full[])() = {Step1_full, Step2_full, Step3_full, Step4_full};

// half step steps
void Step1_half() {
    s_red = 1;
    s_gray = 0;
    s_yellow = 1;
    s_black = 0;
}

void Step2_half() {
    s_red = 1;
    s_gray = 0;
    s_yellow = 0;
    s_black = 0;
}

void Step3_half() {
    s_red = 1;
    s_gray = 0;
    s_yellow = 0;
    s_black = 1;
}

void Step4_half() {
    s_red = 0;
    s_gray = 0;
    s_yellow = 0;
    s_black = 1;
}

void Step5_half() {
    s_red = 0;
    s_gray = 1;
    s_yellow = 0;
    s_black = 1;
}

void Step6_half() {
    s_red = 0;
    s_gray = 1;
    s_yellow = 0;
    s_black = 0;
}

void Step7_half() {
    s_red = 0;
    s_gray = 1;
    s_yellow = 1;
    s_black = 0;
}

void Step8_half() {
    s_red = 0;
    s_gray = 0;
    s_yellow = 1;
    s_black = 0;
}

void (*state_table_half[])() = {Step1_half, Step2_half, Step3_half, Step4_half, Step5_half, Step6_half, Step7_half, Step8_half};

// interrupt service routines
void StepperISR() {
    if (fullstep) { // full step code
        step_full = ++step_full % num_step_full;
        if (cw) {
            state_table_full[(num_step_full - 1) - step_full]();
        } else {
            state_table_full[step_full]();
        }
    } else { // halfstep code
        step_half = ++step_half % num_step_half;
        if (cw) {
            state_table_full[(num_step_half - 1) - step_half]();
        } else {
            state_table_half[step_half]();
        }
    }

    led_1 = !led_1;
}

void toggleCwISR() {
    cw = !cw;
}

void toggleSteps() {
    stepper.detach();
    fullstep = !fullstep;
    time_interval = calcTime(curStudent, fullstep);

    stepper.attach(&StepperISR, std::chrono::milliseconds(time_interval));
}

void increaseSpeed() {
    stepper.detach();
    if (time_interval > 55) time_interval -= 50;
    stepper.attach(&StepperISR, std::chrono::milliseconds(time_interval));
}

void decreaseSpeed() {
    stepper.detach();
    time_interval += 50;
    stepper.attach(&StepperISR, std::chrono::milliseconds(time_interval));
}

int calcTime(int curStudent, int fullstep) {
    if (curStudent) { // lucian
        if (fullstep) {
            return 50.0/48.0 * 1000;
            //return 40;
        } else {
            //return 20;
            return 50.0/96.0 * 1000;
        }
    } else { // melmel
        if (fullstep) {
            //return 100;
            return 40.0/48.0 * 1000;
        } else {
            //return 50;
            return 40.0/96.0 * 1000;
        }
    }
}

int main()
{   
    led_1 = 0;
    LCD.Clear(LCD_COLOR_WHITE);
    while (true) {
        userButton.fall(&switchStudent);

        if (running) {
            updateLCD();

            // shift registers
            shiftRegButton1 = (shiftRegButton1 << 1) + !b1.read();
            shiftRegButton2 = (shiftRegButton2 << 1) + !b2.read();
            shiftRegButton3 = (shiftRegButton3 << 1) + !b3.read();
            shiftRegButton4 = (shiftRegButton4 << 1) + !b4.read();

            if (shiftRegButton1 == DEBOUNCE_PATTERN) {
                printf("button 1 pressed\n");
                toggleCwISR();
            }

            if (shiftRegButton2 == DEBOUNCE_PATTERN) {
                printf("button 2 pressed\n");
                toggleSteps();
            }

            if (shiftRegButton3 == DEBOUNCE_PATTERN) {
                printf("button 3 pressed\n");
                increaseSpeed();
            }

            if (shiftRegButton4 == DEBOUNCE_PATTERN) {
                printf("button 4 pressed\n");
                decreaseSpeed();
            }
        }

        ThisThread::sleep_for(5ms);
    }
}


void switchStudent(void) {
    stepper.detach();
    if (!running) {
        running = 1;
        cw = true;
        fullstep = true;
    }
    curStudent = !curStudent;

    time_interval = calcTime(curStudent, fullstep);

    stepper.attach(&StepperISR, std::chrono::milliseconds(time_interval));
}

void updateLCD(void) {
    LCD.Clear(LCD_COLOR_WHITE);
    uint8_t text[30]; 
    if (curStudent) {
        // Lucian
        sprintf((char *)text, "Lucian Cheng"); 
        LCD.DisplayStringAt(0, 40, (uint8_t *)&text, CENTER_MODE);

        sprintf((char *)text, "400398983");
        LCD.DisplayStringAt(0, 100, (uint8_t *)&text, CENTER_MODE);

        sprintf((char *)text, "Period: 50s");
        LCD.DisplayStringAt(0, 160, (uint8_t *)&text, CENTER_MODE);

    } else {
        // MelMel
        sprintf((char *)text, "Melody Yang"); 
        LCD.DisplayStringAt(0, 40, (uint8_t *)&text, CENTER_MODE);

        sprintf((char *)text, "400307007"); 
        LCD.DisplayStringAt(0, 100, (uint8_t *)&text, CENTER_MODE);

        sprintf((char *)text, "Period: 40s");
        LCD.DisplayStringAt(0, 160, (uint8_t *)&text, CENTER_MODE);
    }

    if (fullstep) {
        sprintf((char *)text, "Full Step");
        LCD.DisplayStringAt(0, 220, (uint8_t *)&text, CENTER_MODE);
    } else {
        sprintf((char *)text, "Half Step");
        LCD.DisplayStringAt(0, 220, (uint8_t *)&text, CENTER_MODE);
    }
}
