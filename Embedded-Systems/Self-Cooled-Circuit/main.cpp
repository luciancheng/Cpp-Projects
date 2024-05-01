#include "LCD_DISCO_F429ZI.h"
#include "TS_DISCO_F429ZI.h"
#include "mbed.h"
#include <stdio.h> 
#define LCD_WIDTH 240
#define LCD_HEIGHT 320

LCD_DISCO_F429ZI LCD;
TS_DISCO_F429ZI TS;

AnalogIn temperatureSensor(PA_0);
PwmOut fan(PD_14);

// function prototypes for states
void state1(void);
void state2(void);
void initializeSM(void);
void update_lcd();
void turnOnFan();

// enumerate states
typedef enum {STATE_1=0, STATE_2} State_Type;

// define a table of pointers to functions for each state
static void (*state_table[]) (void) = {state1, state2};
static State_Type curr_state;

// global variables
float temp;
float thresholdTemp;
int fanSpeed = 0;

int main() {
    initializeSM();
    LCD.SetFont(&Font24);
    LCD.SetTextColor(LCD_COLOR_DARKBLUE);

    TS_StateTypeDef TS_State;
    uint16_t tsX, tsY;

    TS.Init(LCD_WIDTH, LCD_HEIGHT);

    update_lcd();

    while (true) {
        state_table[curr_state]();

        float volt = temperatureSensor.read_voltage();

        TS.GetState(&TS_State);

        temp = volt * 100/3;
        //printf("temp: %.3f\n", temp);
        update_lcd();

        if (TS_State.TouchDetected) {
            tsX = TS_State.X;
            tsY = 320 - TS_State.Y;

            // detect plus button touch
            if ((tsX >= 20) && (tsX <= 20 + LCD_WIDTH/3) && (tsY >= 2*LCD_HEIGHT/3) && (tsY <= 2*LCD_HEIGHT/3 + 75)) {
                thresholdTemp += 0.5;
                LCD.FillRect(20, 2*LCD_HEIGHT/3, LCD_WIDTH/3, 75);
            }

            // detect minus button touch
            if ((tsX >= 140) && (tsX <= 140 + LCD_WIDTH/3) && (tsY >= 2*LCD_HEIGHT/3) && (tsY <= 2*LCD_HEIGHT/3 + 75)) {
                thresholdTemp -= 0.5;
                LCD.FillRect(140, 2*LCD_HEIGHT/3, LCD_WIDTH/3, 75);
            }
        }

        ThisThread::sleep_for(500ms);
    }
}

void update_lcd() {
    LCD.Clear(LCD_COLOR_WHITE);
    uint8_t text[30]; 
    sprintf((char *)text, "Sensor:%0.1fC", temp); //change value sensor shows on LCD
    LCD.DisplayStringAt(0, 40, (uint8_t *)&text, LEFT_MODE);
    sprintf((char *)text, "Thresh:%0.1fC", thresholdTemp); //change value of threshold shows on LCD
    LCD.DisplayStringAt(0, 80, (uint8_t *)&text, LEFT_MODE);

    LCD.SetTextColor(LCD_COLOR_WHITE);
    LCD.DrawRect(20, 2*LCD_HEIGHT/3, LCD_WIDTH/3, 75); //outer boarder for +
    LCD.SetTextColor(LCD_COLOR_DARKBLUE);

    LCD.FillRect(50, 2*LCD_HEIGHT/3, 20, 75); // making | of + 
    LCD.FillRect(20, 240, LCD_WIDTH/3, 20); // making - of +

    LCD.SetTextColor(LCD_COLOR_WHITE);
    LCD.DrawRect(140, 2*LCD_HEIGHT/3, LCD_WIDTH/3, 75); //outer boarder for -
    LCD.SetTextColor(LCD_COLOR_DARKBLUE);
    LCD.FillRect(140, 240, LCD_WIDTH/3, 20); // making -
}

void initializeSM(void) {
    curr_state = STATE_1;
    fan.period_us(100); // set the period to be 100 us
    temperatureSensor.set_reference_voltage(3);

    float volt = temperatureSensor.read_voltage(); // read voltage
    temp = volt * 100/3; // convert voltage reading to deg C

    thresholdTemp = round(temp + 1); // set threshold temperature to be initially greater than the room temperature by 1 degrees C
}

void state1(void) {
    if (temp > thresholdTemp) {
        curr_state = STATE_2;
    }
}

void state2(void) {
    if (temp <= thresholdTemp) {
        curr_state = STATE_1;
        fanSpeed = -10;
    }
    turnOnFan();
}

void turnOnFan() {
    if (fanSpeed < 100) {
        fanSpeed += 10; // incrementally increase fan speed
    }
    fan.pulsewidth_us(fanSpeed); // modulate fan speed
}
