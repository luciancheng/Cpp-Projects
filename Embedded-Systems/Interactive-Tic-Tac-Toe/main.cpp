#include "LCD_DISCO_F429ZI.h"
#include "TS_DISCO_F429ZI.h"
#include "mbed.h"
#include <stdio.h> 
#include <vector>
#include <unordered_map>
#include <string>
#define LCD_WIDTH 240
#define LCD_HEIGHT 320
#define playMusic 0x01

using namespace std;

LCD_DISCO_F429ZI LCD;
TS_DISCO_F429ZI TS;

PwmOut piezo(PA_6);
PwmOut Led_1(PE_8);
PwmOut Led_2(PE_10);
PwmOut Led_3(PE_12);
PwmOut Led_4(PE_14);


Ticker ticker;
Thread t1(osPriorityBelowNormal); // music playing thread
Thread t2(osPriorityBelowNormal); // led thread
EventFlags evt;

// function prototypes
void state1(void); // player 1 turn
void state2(void); // cpu turn
void state3(void); // player wins
void state4(void); // computer wins
void state5(void); // draw
void initializeSM(void);

void updateLCD(void);
int frequency(std::string note);
void turnOffLED(void);
void turnOffLED(void);

// enumerate states
typedef enum {STATE_1=0, STATE_2, STATE_3, STATE_4, STATE_5} State_Type;

// define a table of pointers to functions for each state
static void (*state_table[]) (void) = {state1, state2, state3, state4, state5};
static State_Type curr_state;

// global variables
char gameBoard[] = {' ', ' ', ' ',
                    ' ', ' ', ' ',
                    ' ', ' ', ' '};

int winningPositions[][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};

bool playerTurn = true;
bool gamePlaying = true;
TS_StateTypeDef TS_State;
uint16_t tsX, tsY;
int playsound = 0;
bool evalSituation = false;

// board global variables
int circleSize = 30;
int xSize = 20;
int yOffset = 30;
int boxSize = LCD_WIDTH/3;

// sound effects
std::vector<std::string> notesWin = {"C", "C", "C", "C", "af", "Bf", "C", " ", "Bf", "C"};
std::vector<float> beatsWin = {0.33, 0.33, 0.33, 1, 1, 1, 0.33, 0.33, 0.33, 2};
float bpmWin = 170;
int songLengthWin = notesWin.size();
int beatLengthWin = 60.0/bpmWin * 1000;

std::vector<std::string> notesLose = {"B", "F", "F", "F", "F", "E", "D", "C", "g", "c"};
std::vector<float> beatsLose = {1, 1.5, 0.5, 1, 1, 1, 1, 2, 1, 2};
float bpmLose = 292;
int songLengthLose = notesLose.size();
int beatLengthLose = 60.0/bpmLose * 1000;

std::vector<std::string> notesDraw = {"g", "g", "g", "ds", "af", "af", "af", "Bf", "Bf", "Bf", "Ds"};
std::vector<float> beatsDraw = {0.5, 0.25, 0.25, 1, 0.5, 0.25, 0.25, 0.5, 0.25, 0.25, 2};
float bpmDraw = 160;
int songLengthDraw = notesDraw.size();
int beatLengthDraw = 60.0/bpmDraw * 1000;

std::vector<std::string> notesPlayerMove = {"D", "G"};
std::vector<float> beatsPlayerMove= {0.25, 1.75};
float bpmPlayerMove = 200;
int songLengthPlayerMove = notesPlayerMove.size();
int beatLengthPlayerMove = 60.0/bpmPlayerMove * 1000;

std::vector<std::string> notesCPUMove = {"c", "e", "g", "C"};
std::vector<float> beatsCPUMove= {1, 1, 1, 1};
float bpmCPUMove = 300;
int songLengthCPUMove = notesCPUMove.size();
int beatLengthCPUMove = 60.0/bpmCPUMove * 1000;



// Piezo functions
int frequency(std::string note) {
  std::unordered_map<std::string,int> namefreq = {
      {" ", 0},
      {"a", 440},
      {"bf", 466},
      {"b", 494},
      {"c", 523},
      {"cs", 554},
      {"d", 587},
      {"ds", 622},
      {"e", 659},
      {"f", 698},
      {"fs", 740},
      {"g", 784},
      {"af", 831},
      {"A", 880},
      {"Bf", 932},
      {"B", 988},
      {"C", 1046},
      {"Cs", 1108},
      {"D", 1174},
      {"Ds", 1244},
      {"E", 1318},
      {"F", 1396},
      {"Fs", 1480},
      {"G", 1564},
      {"Af", 1662},
  };

  return namefreq[note];
}

void StopBuzzer() {
    piezo = 0;
}

void BuzzHz(int freq) {
    piezo.period(1.0/freq);
    piezo = 0.5; // set duty cycle
}

void soundFX() {
  // put your main code here, to run repeatedly:
    int i, duration;

    int songLength, beatLength;
    std::vector<std::string> notes;
    std::vector<float> beats;

    if (playsound == 1) { // victory sound
        songLength = songLengthWin;
        beatLength = beatLengthWin;
        notes = notesWin;
        beats = beatsWin;
    } else if (playsound == 2) { // lose sound
        songLength = songLengthLose;
        beatLength = beatLengthLose;
        notes = notesLose;
        beats = beatsLose;
    } else if (playsound == 3) {
        songLength = songLengthDraw;
        beatLength = beatLengthDraw;
        notes = notesDraw;
        beats = beatsDraw;
    } else if (playsound == 4) {
        songLength = songLengthPlayerMove;
        beatLength = beatLengthPlayerMove;
        notes = notesPlayerMove;
        beats = beatsPlayerMove;
    } else if (playsound == 5) {
        songLength = songLengthCPUMove;
        beatLength = beatLengthCPUMove;
        notes = notesCPUMove;
        beats = beatsCPUMove;
    }
    if (playsound != 0) {
        for (i = 0; i < songLength; i++) {
            duration = beats[i]*beatLength;

            if (notes[i] == " ")
            {
                StopBuzzer();
                ThisThread::sleep_for(duration);
            }
            else
            {
                BuzzHz(frequency(notes[i]));
                //tone(Buzzer, frequency(notes[i]), duration);
                ThisThread::sleep_for(duration);
            }
            ThisThread::sleep_for(beatLength/7.0);
            // brief pause between notes
        }
  }
  StopBuzzer();
}

// buzzer thread
void thread1(void) {
    while (true) {
        evt.wait_any(playMusic, osWaitForever);
        soundFX();
        playsound = 0;
        evt.clear(playMusic);
    }
}

int LEDCounter = 0;
int LEDPattern = 0;
// led thread
void thread2(void) {
    bool inc = true;
    int output = 255;
    while (true) {
        if (LEDPattern == 0) {
            if (inc) {
                LEDCounter += 1;
                if (LEDCounter == 4) {
                    inc = false;
                }
            } else {
                LEDCounter -= 1;
                if (LEDCounter == 1) {
                    inc = true;
                }
            }
            switch(LEDCounter) {
                    case(4):
                        turnOffLED();
                        Led_4.pulsewidth_us(255);
                        break;
                    case(3):
                        turnOffLED();
                        Led_3.pulsewidth_us(255);
                        break;
                    case(2):
                        turnOffLED();
                        Led_2.pulsewidth_us(255);
                        break;
                    case(1):
                        turnOffLED();
                        Led_1.pulsewidth_us(255);
                        break;
            }
        } else if (LEDPattern == 1) {
            turnOffLED();
            Led_4.pulsewidth_us(255);
            Led_3.pulsewidth_us(255);
            ThisThread::sleep_for(100ms);
            Led_2.pulsewidth_us(255);
            Led_1.pulsewidth_us(255);
            ThisThread::sleep_for(800ms);
            LEDPattern = 2;
        } else if (LEDPattern == 2) {
            turnOffLED();
            Led_4.pulsewidth_us(255);
            ThisThread::sleep_for(250ms);
            Led_3.pulsewidth_us(255);
            ThisThread::sleep_for(250ms);
            Led_2.pulsewidth_us(255);
            ThisThread::sleep_for(250ms);
            Led_1.pulsewidth_us(255);
            ThisThread::sleep_for(250ms);
            LEDPattern = 0;
        } else if (LEDPattern == 3) {
            // Can win
            for (int i = 0; i < 256; i++) {
                Led_1.pulsewidth_us(i);
                Led_2.pulsewidth_us(i);
                Led_3.pulsewidth_us(i);
                Led_4.pulsewidth_us(i);
                ThisThread::sleep_for(1ms);
            }

            for (int i = 255; i >= 0; i--) {
                Led_1.pulsewidth_us(i);
                Led_2.pulsewidth_us(i);
                Led_3.pulsewidth_us(i);
                Led_4.pulsewidth_us(i);
                ThisThread::sleep_for(1ms);
            }
        } else if (LEDPattern == 4) {
            if (output == 255) {
                output = 0;
            } else {
                output = 255;
            }
            Led_1.pulsewidth_us(output);
            Led_2.pulsewidth_us(output);
            Led_3.pulsewidth_us(output);
            Led_4.pulsewidth_us(output);
            ThisThread::sleep_for(250ms);
        } else if (LEDPattern == 5) {
            turnOffLED();
        }
        ThisThread::sleep_for(100ms);
    }
}

int main() {
    piezo = 0;
    initializeSM();
    t1.start(thread1);
    t2.start(thread2);

    while (true) {
        updateLCD();
        state_table[curr_state]();
        ThisThread::sleep_for(150ms);
    }
}

void updateLCD(void) {
    LCD.Clear(LCD_COLOR_WHITE);
    uint8_t text[30]; 
    sprintf((char *)text, "Tic-Tac-Toe"); 
    LCD.DisplayStringAt(0, 10, (uint8_t *)&text, CENTER_MODE);

    // touch boxes
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++) {
            LCD.SetTextColor(LCD_COLOR_WHITE);
            LCD.DrawRect(boxSize * j, yOffset + i * boxSize, boxSize, boxSize);
        }
    }

    LCD.SetTextColor(LCD_COLOR_DARKBLUE);
    // draw grid lines
    LCD.FillRect(boxSize - 2, yOffset, 5, boxSize * 3);
    LCD.FillRect(boxSize * 2 - 2, yOffset, 5, boxSize * 3);
    LCD.FillRect(0, yOffset + boxSize - 2, boxSize * 3, 5);
    LCD.FillRect(0, yOffset + 2*boxSize - 2, boxSize * 3, 5);

    // draw moves

    // circle
    int xCenterCoor[] = {boxSize/2, boxSize + boxSize/2, 2*boxSize + boxSize/2};
    int yCenterCoor[] = {yOffset + boxSize/2, yOffset + boxSize + boxSize/2, yOffset + 2*boxSize + boxSize/2};

    int j = 0;
    for (int i = 0; i < 9; i++) {
        if ((i > 0) && (i % 3 == 0)) {
            j++;
        }
        int xCoor = xCenterCoor[i % 3];
        int yCoor = yCenterCoor[j];

        if (gameBoard[i] == 'X') {
            LCD.DrawLine(xCoor - xSize, yCoor - xSize, xCoor + xSize, yCoor + xSize);
            LCD.DrawLine(xCoor + xSize, yCoor - xSize, xCoor - xSize, yCoor + xSize);
        } else if (gameBoard[i] == 'O') {
            LCD.DrawCircle(xCoor, yCoor, circleSize);
        }
    }

    if (gamePlaying) {
        if (playerTurn) {
            sprintf((char *)text, "Your Turn");
        } else {
            sprintf((char *)text, "Computer Turn");
        }
        LCD.DisplayStringAt(0, 290, (uint8_t *)&text, CENTER_MODE);
    } else {
        LCD.SetTextColor(LCD_COLOR_RED);
        LCD.FillRect(LCD_WIDTH/3 - boxSize/2, 210, boxSize * 2, boxSize*3/4);
        LCD.SetTextColor(LCD_COLOR_WHITE);
        LCD.SetBackColor(LCD_COLOR_RED);
        sprintf((char *)text, "Play Again");
        LCD.DisplayStringAt(0, 230, (uint8_t *)&text, CENTER_MODE);
        LCD.SetBackColor(LCD_COLOR_WHITE);
        LCD.SetTextColor(LCD_COLOR_DARKBLUE);


    }
}

void resetGame() {
    curr_state = STATE_1;
    for (int i = 0; i < 9; i++) {
        gameBoard[i] = ' ';
    }
    playerTurn = true;
    gamePlaying = true;
    LEDPattern = 0;
}

bool isOccupied(char board[], int index) {
    if (board[index] == ' ') {
        return false;
    } else {
        return true;
    }
}

int processCpuMove(char board[]) {
    // win if possible
    vector<bool> cpuOccupiedSpots(9, false);
    for (int i = 0; i < 9; i++) {
        if (board[i] == 'O') {
            cpuOccupiedSpots[i] = true;
        }
    }
    
    int count = 0;
    for (auto & pos : winningPositions) {
        int cpuMove = 0;
        for (int i = 0; i < 3; i++) {
            if (cpuOccupiedSpots[pos[i]]) {
                count++;
            } else {
                cpuMove = pos[i];
            }
        }
        if (count == 2) {
            if (!isOccupied(board, cpuMove)) return cpuMove;
        }
        count = 0;
    }

    // block player

    vector<bool> playerOccupiedSpots(9, false);
    for (int i = 0; i < 9; i++) {
        if (board[i] == 'X') {
            playerOccupiedSpots[i] = true;
        }
    }
    
    int count2 = 0;
    for (auto & pos : winningPositions) {
        int cpuMove = 0;
        for (int i = 0; i < 3; i++) {
            if (playerOccupiedSpots[pos[i]]) {
                count2++;
            } else {
                cpuMove = pos[i];
            }
        }
        if (count2 == 2) {
            if (!isOccupied(board, cpuMove)) return cpuMove;
        }
        count2 = 0;
    }

    // take middle

    if (!isOccupied(board, 4)) {
        return 4;
    }

    // random move
    srand((unsigned) time(NULL));

    int cpuMove = rand() % 9; // between 1000ms and 5000ms
    while (isOccupied(board, cpuMove)) {
        cpuMove = rand() % 9;
    }

    return cpuMove;
}

bool checkWin(char board[], char player) {
    for (auto & pos : winningPositions) {
        if ((board[pos[0]] == player) && (board[pos[1]] == player) && (board[pos[2]] == player)) {
            return true;
        }
    }
    return false;
}

bool isDraw(char board[]) {
    for (int i = 0; i < 9; i++) {
        if (board[i] == ' ') {
            return false;
        }
    }
    return true;
}

void turnOffLED(void) {
    Led_1.pulsewidth_us(0);
    Led_2.pulsewidth_us(0);
    Led_3.pulsewidth_us(0);
    Led_4.pulsewidth_us(0);
}

void initializeSM(void) {
    curr_state = STATE_1;

    LCD.SetFont(&Font20);
    LCD.SetTextColor(LCD_COLOR_DARKBLUE);
    TS.Init(LCD_WIDTH, LCD_HEIGHT);

    piezo = 0;

    evalSituation = true;
    LEDPattern = 0;

    Led_1.period_us(255);
    Led_2.period_us(255);
    Led_3.period_us(255);
    Led_4.period_us(255);
    turnOffLED();
}

void state1(void) {
    int boardPressed = 0;

    if (evalSituation) {
        // check if in danger
        vector<bool> cpuOccupiedSpots(9, false);
        for (int i = 0; i < 9; i++) {
            if (gameBoard[i] == 'O') {
                cpuOccupiedSpots[i] = true;
            }
        }
        
        int count = 0;
        for (auto & pos : winningPositions) {
            int cpuMove = 0;
            for (int i = 0; i < 3; i++) {
                if (cpuOccupiedSpots[pos[i]]) {
                    count++;
                } else {
                    cpuMove = pos[i];
                }
            }
            if (count == 2) {
                if (!isOccupied(gameBoard, cpuMove)) {
                    printf("DANGER\n");
                    LEDPattern = 4;
                }
            }
            count = 0;
        }


        // win if possible
        vector<bool> playerOccupiedSpots(9, false);
        for (int i = 0; i < 9; i++) {
            if (gameBoard[i] == 'X') {
                playerOccupiedSpots[i] = true;
            }
        }
        
        int count2 = 0;
        for (auto & pos : winningPositions) {
            int playerMove = 0;
            for (int i = 0; i < 3; i++) {
                if (playerOccupiedSpots[pos[i]]) {
                    count2++;
                } else {
                    playerMove = pos[i];
                }
            }
            if (count2 == 2) {
                if (!isOccupied(gameBoard, playerMove)) {
                    printf("WINNABLE\n");
                    LEDPattern = 3;
                }
            }
            count2 = 0;
        }
        evalSituation = false;
    }
    TS.GetState(&TS_State);

    if (TS_State.TouchDetected) {
        tsX = TS_State.X;
        tsY = LCD_HEIGHT - TS_State.Y;

        // iterate through each box to see if the touch is detected within any of these boxes
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if ((tsX >= boxSize * j) && (tsX <= boxSize * j + boxSize) && (tsY >= yOffset + i * boxSize) && (tsY <= yOffset + i * boxSize + boxSize)) {
                    int boardIndex = j + i * 3;

                    if (!isOccupied(gameBoard, boardIndex)) {
                        gameBoard[boardIndex] = 'X';
                        curr_state = STATE_2;
                        playerTurn = false;
                        boardPressed = 1;
                    }
                }
            }
        }
    }

    if (checkWin(gameBoard, 'X')) {
        curr_state = STATE_3;
        gamePlaying = false;
        playsound = 1;
        evt.set(playMusic);
        LEDPattern = 5;
        return;
    }

    if (isDraw(gameBoard)) {
        curr_state = STATE_5;
        gamePlaying = false;
        playsound = 3;
        evt.set(playMusic);
        LEDPattern = 5;
        return;
    }
    
    if (boardPressed) {
        playsound = 4;
        LEDPattern = 1;
        evt.set(playMusic);
        boardPressed = 0;
    }
}

void state2(void) {
    ThisThread::sleep_for(1000ms);
    int cpuMove = processCpuMove(gameBoard);
    updateLCD();
    playsound = 5;
    evt.set(playMusic);

    ThisThread::sleep_for(1000ms);
    gameBoard[cpuMove] = 'O';

    //ThisThread::sleep_for(200ms);

    if (checkWin(gameBoard, 'O')) {
        curr_state = STATE_4;
        gamePlaying = false;
        playsound = 2;
        evt.set(playMusic);
        LEDPattern = 5;
        return;
    }

    if (isDraw(gameBoard)) {
        curr_state = STATE_5;
        gamePlaying = false;
        playsound = 3;
        evt.set(playMusic);
        LEDPattern = 5;
        return;
    }

    curr_state = STATE_1;
    playerTurn = true;
    LEDPattern = 0;
    evalSituation = true;
}

void state3(void) {
    LEDPattern = 5;
    uint8_t text[30]; 
    sprintf((char *)text, "YOU WIN!");
    LCD.SetTextColor(LCD_COLOR_WHITE);
    LCD.SetBackColor(LCD_COLOR_RED);
    LCD.DisplayStringAt(0, LCD_HEIGHT/2 - 25, (uint8_t *)&text, CENTER_MODE);
    LCD.SetTextColor(LCD_COLOR_DARKBLUE);
    LCD.SetBackColor(LCD_COLOR_WHITE);

    TS.GetState(&TS_State);

    if (TS_State.TouchDetected) {
        tsX = TS_State.X;
        tsY = LCD_HEIGHT - TS_State.Y;

        if ((tsX >= LCD_WIDTH/3 - boxSize/2) && (tsX <= LCD_WIDTH/3 - boxSize/2 + 2*boxSize) && (tsY >= 210) && (tsY <= 210 + boxSize)) {
            resetGame();
        }
    }
}

void state4(void) {
    LEDPattern = 5;
    uint8_t text[30]; 
    sprintf((char *)text, "YOU LOST :(");
    LCD.SetTextColor(LCD_COLOR_WHITE);
    LCD.SetBackColor(LCD_COLOR_RED);
    LCD.DisplayStringAt(0, LCD_HEIGHT/2 - 25, (uint8_t *)&text, CENTER_MODE);
    LCD.SetTextColor(LCD_COLOR_DARKBLUE);
    LCD.SetBackColor(LCD_COLOR_WHITE);
    
    TS.GetState(&TS_State);

    if (TS_State.TouchDetected) {
        tsX = TS_State.X;
        tsY = LCD_HEIGHT - TS_State.Y;

        if ((tsX >= LCD_WIDTH/3 - boxSize/2) && (tsX <= LCD_WIDTH/3 - boxSize/2 + 2*boxSize) && (tsY >= 210) && (tsY <= 210 + boxSize)) {
            resetGame();
        }
    }
}

void state5(void) {
    LEDPattern = 5;
    uint8_t text[30]; 
    sprintf((char *)text, "DRAW");
    LCD.SetTextColor(LCD_COLOR_WHITE);
    LCD.SetBackColor(LCD_COLOR_RED);
    LCD.DisplayStringAt(0, LCD_HEIGHT/2 - 25, (uint8_t *)&text, CENTER_MODE);
    LCD.SetTextColor(LCD_COLOR_DARKBLUE);
    LCD.SetBackColor(LCD_COLOR_WHITE);

    TS.GetState(&TS_State);

    if (TS_State.TouchDetected) {
        tsX = TS_State.X;
        tsY = LCD_HEIGHT - TS_State.Y;

        if ((tsX >= LCD_WIDTH/3 - boxSize/2) && (tsX <= LCD_WIDTH/3 - boxSize/2 + 2*boxSize) && (tsY >= 210) && (tsY <= 210 + boxSize)) {
            resetGame();
        }
    }
}
