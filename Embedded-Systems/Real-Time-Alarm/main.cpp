#include "LCD_DISCO_F429ZI.h"
#include "mbed.h"
#include <time.h>
#include <string.h>
#include <cstdlib>
#include <vector> 

#define SDA_PIN PC_9 //EEPROM for memory
#define SCL_PIN PA_8 //clock 
#define EEPROM_ADDR 0xA0 // kelly gave this
#define DEBOUNCE_PATTERN 0x7FFF //debounce


LCD_DISCO_F429ZI LCD;

I2C i2c(SDA_PIN, SCL_PIN);

uint16_t shiftRegButton1; //button 1
uint16_t shiftRegButton2; //button 2
uint16_t shiftRegButton3; //button 3

void WriteEEPROM(int address, unsigned int ep_address, char *data, int size);
void ReadEEPROM(int address, unsigned int ep_address, char *data, int size);

// function prototypes for states
void state1(void);
void state2(void);
void state3(void);
void state4(void);
void state5(void);
void initializeSM(void);
void save_time(void);
void displayEditingTimes(int i);

// enumerate states
typedef enum {STATE_1=0, STATE_2, STATE_3, STATE_4, STATE_5} State_Type;

// define a table of pointers to functions for each state
static void (*state_table[]) (void) = {state1, state2, state3, state4, state5};
static State_Type curr_state;

// button declaration
InterruptIn b1(PA_6, PullUp); // button for display
InterruptIn b2(PA_7, PullUp); // cycle button
InterruptIn b3(PA_5, PullUp); // increase button
InterruptIn userButton(BUTTON1); // user button

// global vairable declarations
char timebuf[20];
int saveTimeToEeprom = 0;
int displayTimes = 0;
int lenTime = 8;
int curtime = 0;
unsigned int eeprom_addr = 0;
int getCurTime = 0;
vector<long> timeInInt;
tm t;

int main() {

  // Read and write a byte from/to EEPROM lulututu

  char data_read[10];
  ReadEEPROM(EEPROM_ADDR, eeprom_addr, data_read, 10);
  printf("Previous value in EEPROM: %s\n", data_read);
   
  char data_write[20] = "00:00:00 00:00:00";
  WriteEEPROM(EEPROM_ADDR, eeprom_addr, data_write, 20);
  printf("Wrote value to EEPROM: %s\n", data_write);//*/

  // set RTC to January 1, 2024, 00:00:00 (HH:MM:SS)
  initializeSM();

  while (1) {
    // update time
    time_t rawtime;
    time(&rawtime);
    strftime(timebuf, 20, "%H:%M:%S", localtime(&rawtime));

    // Read RTC and display on LCD
    state_table[curr_state]();

    // Button Presses

    shiftRegButton1 = (shiftRegButton1 << 1) + !b1.read(); // reading button 1
    shiftRegButton2 = (shiftRegButton2 << 1) + !b2.read();
    shiftRegButton3 = (shiftRegButton3 << 1) + !b3.read();

    userButton.fall(&save_time);

    if (saveTimeToEeprom) {
        char previousData[20];
        ReadEEPROM(EEPROM_ADDR, eeprom_addr, previousData, 20);

        // edit the save string 
        char writedata[20];
        int i = 0;
        int k = 0;

        // get the most recent save, then add the new time to it
        while (previousData[k] != ' ') {
            k++;
        }
        k++;

        while (previousData[k] != '\0') {
            writedata[i] = previousData[k];
            i++;
            k++;
        }
        
        writedata[i] = ' ';
        i++;
  
        for (int j = 0; timebuf[j] != '\0'; j++) {
            writedata[i] = timebuf[j];
            i++;
        }
        writedata[i] = '\0';

        WriteEEPROM(EEPROM_ADDR, eeprom_addr, writedata, 20);
        saveTimeToEeprom = 0;
    }

    curtime++;
    thread_sleep_for(1);
  }
}

// This function has 63 bytes write limit
void WriteEEPROM(int address, unsigned int eeaddress, char *data, int size) {
  char i2cBuffer[size + 2];
  i2cBuffer[0] = (unsigned char)(eeaddress >> 8);   // MSB
  i2cBuffer[1] = (unsigned char)(eeaddress & 0xFF); // LSB

  for (int i = 0; i < size; i++) {
    i2cBuffer[i + 2] = data[i];
  }

  int result = i2c.write(address, i2cBuffer, size + 2, false);
  thread_sleep_for(6);
}

void ReadEEPROM(int address, unsigned int eeaddress, char *data, int size) {
  char i2cBuffer[2];
  i2cBuffer[0] = (unsigned char)(eeaddress >> 8);   // MSB
  i2cBuffer[1] = (unsigned char)(eeaddress & 0xFF); // LSB

  // Reset eeprom pointer address
  int result = i2c.write(address, i2cBuffer, 2, false);
  thread_sleep_for(6);

  // Read eeprom
  i2c.read(address, data, size);
  thread_sleep_for(6);
}

void printVector(vector<long> vec) {
    for (auto & i : vec) {
        printf("%ld ", i);
    }
    printf("\n");
}

void initializeSM(void) {
    curr_state = STATE_1;

    // change these values when setting time
    // set the current time
    t.tm_year = 124; // years since 1900
    t.tm_mon = 0;
    t.tm_mday = 0;
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;
    set_time(mktime(&t));
}

void state1(void) {
    if (curtime >= 1000) {
        LCD.Clear(LCD_COLOR_WHITE);
        LCD.SetFont(&Font24);
        LCD.SetTextColor(LCD_COLOR_BLACK);
        LCD.SetBackColor(LCD_COLOR_WHITE);  // highlight the text
        char text[20] = "HH:MM:SS";
        LCD.DisplayStringAt(0, 30, (uint8_t *)&text, CENTER_MODE);
        LCD.SetBackColor(LCD_COLOR_ORANGE);  // highlight the text
        LCD.DisplayStringAt(0, 80, (uint8_t *)&timebuf, CENTER_MODE);
        curtime = 0;
    }

    if (shiftRegButton1 == DEBOUNCE_PATTERN) {
        curr_state = STATE_2;
        displayTimes = 1;
    }
    
    if (shiftRegButton3 == DEBOUNCE_PATTERN) {
        curr_state = STATE_3;
        getCurTime = 1;
        displayTimes = 1;
    }
}

void state2(void) {
    if (displayTimes) {
        char data_eeprom[20];
        ReadEEPROM(EEPROM_ADDR, eeprom_addr, data_eeprom, 20);

        LCD.Clear(LCD_COLOR_WHITE);
        LCD.SetBackColor(LCD_COLOR_WHITE);
        char text[20] = "Saved Times";
        LCD.DisplayStringAt(0, 30, (uint8_t *)&text, CENTER_MODE);

        // parse times
        char time1[10];
        char time2[10];

        int i = 0;
        int j = 0;
        while(data_eeprom[i] != ' ') {
            time1[i] = data_eeprom[i];
            i++;
        }
        time1[i] = '\0';
        i++;
        while(data_eeprom[i] != '\0'){
            time2[j] = data_eeprom[i];
            i++;
            j++;
        }
        time2[j] = '\0';

        // display the times on the screen
        LCD.SetTextColor(LCD_COLOR_BLACK);
        LCD.DisplayStringAt(0, 80, (uint8_t *)&time2, CENTER_MODE);

        LCD.DisplayStringAt(0, 130, (uint8_t *)&time1, CENTER_MODE);
        displayTimes = 0;
    }
    if (shiftRegButton1 == DEBOUNCE_PATTERN) {
        curr_state = STATE_1;
    }
}

void goBackToState1() {
    curr_state = STATE_1;


    t.tm_year = 124; // years since 1900
    t.tm_mon = 0;
    t.tm_mday = 0;
    t.tm_hour = timeInInt[0];
    t.tm_min = timeInInt[1];
    t.tm_sec = timeInInt[2];
    set_time(mktime(&t));

    timeInInt.clear();
}

void state3(void) { // display time (hours)
    // need to extract current displayed time and allow for edits
    if (getCurTime) {
        char curTime[20];
        strncpy(curTime, timebuf, 20);
        char * dump;

        char * token = strtok(curTime, ":");
        while (token) {
            timeInInt.push_back(strtol(token, &dump, 10));
            token = strtok(NULL, ":");
        }
        printVector(timeInInt);
        
        getCurTime = 0;
    }

    if (displayTimes) {
        displayEditingTimes(0);
        displayTimes = 0;
    }

    // increase set number
    if (shiftRegButton1 == DEBOUNCE_PATTERN) {
        timeInInt[0] = (timeInInt[0] + 1) % 24; // go up to 23 hours before returning back to 0
        printVector(timeInInt);
        displayEditingTimes(0);
    }

    // go back to idle state
    if (shiftRegButton3 == DEBOUNCE_PATTERN) {
        goBackToState1();
    }

    // switch to editing minutes
    if (shiftRegButton2 == DEBOUNCE_PATTERN) {
        curr_state = STATE_4;
        displayTimes = 1;
    }

}

void state4(void) {
    if (displayTimes) {
        displayEditingTimes(1);
        displayTimes = 0;
    }

    // increase set number
    if (shiftRegButton1 == DEBOUNCE_PATTERN) {
        timeInInt[1] = (timeInInt[1] + 1) % 60; // go up to 59 minutes
        printVector(timeInInt);
        displayEditingTimes(1);
    }

    //cycle editing
    if (shiftRegButton2 == DEBOUNCE_PATTERN) {
        curr_state = STATE_5;
         displayTimes = 1;
    }

    // go back to idle state
    if (shiftRegButton3 == DEBOUNCE_PATTERN) {
        goBackToState1();
    }
}

void state5(void) {
    if (displayTimes) {
        displayEditingTimes(2);
        displayTimes = 0;
    }

    // increase set number
    if (shiftRegButton1 == DEBOUNCE_PATTERN) {
        timeInInt[2] = (timeInInt[2] + 1) % 60; // go up to 59 seconds
        printVector(timeInInt);
        displayEditingTimes(2);
    }
    
    // cycle editing
    if (shiftRegButton2 == DEBOUNCE_PATTERN) {
        curr_state = STATE_3;
        displayTimes = 1;
    }

    // go back to idle state
    if (shiftRegButton3 == DEBOUNCE_PATTERN) {
        goBackToState1();
    }
}

void save_time() {
    saveTimeToEeprom = 1;
}


void displayEditingTimes(int i) { //so screen does NOT flash during editting
    LCD.Clear(LCD_COLOR_WHITE);
    LCD.SetFont(&Font24);
    LCD.SetTextColor(LCD_COLOR_BLACK);
    LCD.SetBackColor(LCD_COLOR_WHITE);  // highlight the text
    char text[20];
    sprintf((char *)text, "HOURS");
    LCD.DisplayStringAt(0, 30, (uint8_t *)&text, CENTER_MODE);
    sprintf((char *)text, "MINUTES");
    LCD.DisplayStringAt(0, 130, (uint8_t *)&text, CENTER_MODE);
    sprintf((char *)text, "SECONDS");
    LCD.DisplayStringAt(0, 230, (uint8_t *)&text, CENTER_MODE);
    //LCD.SetBackColor(LCD_COLOR_ORANGE);  // highlight the text
    //LCD.DisplayStringAt(0, 80, (uint8_t *)&timebuf, CENTER_MODE);

    if (i == 0) { //changing hours
        LCD.SetBackColor(LCD_COLOR_ORANGE);  // highlight the text
        sprintf((char *)text, "%ld", timeInInt[0]);
        LCD.DisplayStringAt(0, 80, (uint8_t *)&text, CENTER_MODE);
        LCD.SetBackColor(LCD_COLOR_WHITE);  // highlight the text
        sprintf((char *)text, "%ld", timeInInt[1]);
        LCD.DisplayStringAt(0, 180, (uint8_t *)&text, CENTER_MODE);
        sprintf((char *)text, "%ld", timeInInt[2]);
        LCD.DisplayStringAt(0, 280, (uint8_t *)&text, CENTER_MODE);
    } else if (i == 1) { //changing minutes
        LCD.SetBackColor(LCD_COLOR_WHITE);  // highlight the text
        sprintf((char *)text, "%ld", timeInInt[0]);
        LCD.DisplayStringAt(0, 80, (uint8_t *)&text, CENTER_MODE);
        LCD.SetBackColor(LCD_COLOR_ORANGE);  // highlight the text
        sprintf((char *)text, "%ld", timeInInt[1]);
        LCD.DisplayStringAt(0, 180, (uint8_t *)&text, CENTER_MODE);
        LCD.SetBackColor(LCD_COLOR_WHITE);
        sprintf((char *)text, "%ld", timeInInt[2]);
        LCD.DisplayStringAt(0, 280, (uint8_t *)&text, CENTER_MODE);
      
    } else { //changing seconds
        LCD.SetBackColor(LCD_COLOR_WHITE);  // highlight the text
        sprintf((char *)text, "%ld", timeInInt[0]);
        LCD.DisplayStringAt(0, 80, (uint8_t *)&text, CENTER_MODE);
        LCD.SetBackColor(LCD_COLOR_WHITE);  // highlight the text
        sprintf((char *)text, "%ld", timeInInt[1]);
        LCD.DisplayStringAt(0, 180, (uint8_t *)&text, CENTER_MODE);
        LCD.SetBackColor(LCD_COLOR_ORANGE);
        sprintf((char *)text, "%ld", timeInInt[2]);
        LCD.DisplayStringAt(0, 280, (uint8_t *)&text, CENTER_MODE);
    }
}