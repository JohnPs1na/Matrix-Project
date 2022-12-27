#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

using namespace std;

#define LEFT 1
#define DOWN 2
#define RIGHT 3
#define UP 4

#define MENU 1
#define OVER 3


#define ABOUT 5
#define GAME 6
#define SETTINGS 7
#define LEADERBOARD 8

#define MATRIX_SETTINGS 0
#define DISPLAY_SETTINGS 1
#define DIFFICULTY_SETTINGS 2


#define MATRIX_BRIGHTNESS 4
#define DISPLAY_CONTRAST 5
#define DIFFICULTY 6

bool playButtonDrawn = false;
bool settingsKeyDrawn = false;
bool aboutDrawn = false;
bool leaderboardDrawn = false;
bool fullMatrixDrawn = false;

int diffSpeed = 375;

String alphabet = "abcdefghijklmnopqrstuvwxyz";

int insertedLetters[] = {0,0,0};
int letterIndex = 0;



int snakeScore = 0;

int menuOption = 1;
int lastMenuOption = menuOption;


int settings[] = {2,2,2};
byte squareLCD[8]={
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

byte fullMatrix[] = {
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
};
byte playButton[] ={
  B00000000,
  B01111110,
  B01111110,
  B01111110,
  B01111110,
  B00111100,
  B00011000,
  B00000000
};
byte settingsKey[] = {
  B11000000,
  B11000000,
  B00100000,
  B00011000,
  B00011110,
  B00001001,
  B00001000,
  B00000100
};

byte leaderboard[] = {
  B00011111,
  B10000011,
  B10001110,
  B11111100,
  B11111100,
  B10001110,
  B10000011,
  B00011111
};

byte about[] = {
  B00000000,
  B00100000,
  B00100110,
  B00100000,
  B00100000,
  B00100110,
  B00100000,
  B00000000
};

byte imagesArray[8][4] = {playButton,settingsKey,leaderboard,about};
bool thinghies[5] = {false,&playButtonDrawn,&settingsKeyDrawn,&aboutDrawn,&leaderboardDrawn};

const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte matrixSize = 8;

const byte RS = 9;
const byte enable = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
const byte lcdContrast = A3;

int contrastValue = 30;

char playerName[3] = "den";

const int pinSW = 2;
const int pinX = A0;
const int pinY = A1;
byte swState = LOW;
int xValue = 0;
int yValue = 0;

const int minThreshold = 400;
const int maxThreshold = 600;
bool joyMoved = false;

bool val = true;

LedControl lc = LedControl(dinPin,clockPin,loadPin,1);
LiquidCrystal lcd(RS,enable,d4,d5,d6,d7);

byte matrixBrightness = 5;


int GAME_WIDTH = 8;
int GAME_HEIGHT = 8;
int SPEED = 250;
int SPACE_SIZE = 1;
int BODY_PARTS = 2;

int score = 0;
int direction = DOWN;

unsigned long timePassed = 0;

bool gameOver = false;

int mainState = MENU;

int lastSwState = 0;
unsigned long lastDebounceTime = 0;
int reading = 0;

int buttonPressInterval = 50;

const int scoreAddress1 = 0;
const int scoreAddress2 = 32;
const int scoreAddress3 = 64;

struct highscore{
  char name[3] = "aaa";
  int score = 0;
};

void clearMatrix(){
  for (int i = 0;i<matrixSize;i++){
    for (int j = 0;j<matrixSize;j++){
      lc.setLed(0,i,j,false);
    }
  }  
}

int snakeCoords[64][2] = {{-1,-1}};
int fakeSnakeCoords[64][2] = {{-1,-1}};

// food related
int foodCoords[2] = {0};
bool foodIn = false;

void createFood(){
  foodCoords[0] = (int)random(0,8);
  foodCoords[1] = (int)random(0,8);
  lc.setLed(0, foodCoords[0], foodCoords[1], true);
} 
void removeFood() {
  lc.setLed(0, foodCoords[0], foodCoords[1], false);
  foodIn = false;
}

int lastScore = 0;
//game related
void next_turn(){ 
  int x = snakeCoords[0][0];
  int y = snakeCoords[0][1];

  if (direction == UP) {
    y-=1;
  } else if (direction == DOWN) {
    y+=1;
  } else if (direction == RIGHT) {
    x+=1;
  } else if (direction == LEFT){
    x-=1;
  }


  int idx = 0;

  while (true) {
    if (snakeCoords[idx][0] == -1){
      break;
    }
    idx++;
  }

  for (int i = idx;i>0;i--){
    snakeCoords[i][0] = snakeCoords[i-1][0];
    snakeCoords[i][1] = snakeCoords[i-1][1];
  }

  snakeCoords[0][0] = x;
  snakeCoords[0][1] = y;


  if(x == foodCoords[0] && y == foodCoords[1]){
    snakeScore++;
    removeFood();
  } else {
    lc.setLed(0, snakeCoords[idx][0], snakeCoords[idx][1], false);

    snakeCoords[idx][0] = -1;
    snakeCoords[idx][1] = -1;
  }
  
  lc.setLed(0, x, y, true);

  if (checkCollisions()){
    lastScore = snakeScore;
    foodIn = false;
    mainState = OVER;
  }
}
bool checkCollisions(){
  int x = snakeCoords[0][0];
  int y = snakeCoords[0][1];

  if (x < 0 || x >= 8){
    return true;
  }
  if (y < 0 || y >= 8) {
    return true;
  }

  int idx = 0;

  while (true) {
    if (snakeCoords[idx][0] == -1){
      break;
    }
    idx++;
  }

  for (int i = 1;i<idx;i++){
    if (x == snakeCoords[i][0] && y == snakeCoords[i][1]){
      return true;
    }
  }

  return false;
}
void control_joystick() {
  if (joyMoved == false) {
    if(xValue < minThreshold){
      if (direction != DOWN)
        direction = UP;
    } 
    if(xValue > maxThreshold){
      if (direction != UP)
        direction = DOWN;
    }
    if(yValue < minThreshold){
      if(direction != LEFT)
        direction = RIGHT;
    }
    if(yValue > maxThreshold){
      if(direction != RIGHT)
        direction = LEFT;
    }
    joyMoved = true;
  }

  if (xValue > minThreshold && xValue < maxThreshold && yValue > minThreshold && yValue < maxThreshold) {
    joyMoved = false;
  }
}

void createFakeSnake() {
  direction = LEFT;

    // setup snake 
  for(int i = 0;i<64;i++){
    fakeSnakeCoords[i][0] = -1;
    fakeSnakeCoords[i][1] = -1;
  }

  
  for(int i = 0;i<4;i++){
    fakeSnakeCoords[i][0] = 7;
    fakeSnakeCoords[i][1] = 4;
  }

  for (int i = 0;i<64;i++){
    if (fakeSnakeCoords[i][0] != -1){
      lc.setLed(0, fakeSnakeCoords[i][0], fakeSnakeCoords[i][1],true);
    }
  }
}

void nextFakeTurn(){ 
  int x = fakeSnakeCoords[0][0];
  int y = fakeSnakeCoords[0][1];

  if(x < 0){
    x = 7;
    fakeSnakeCoords[0][0] = 7;
  }
  if (direction == UP) {
    y-=1;
  } else if (direction == DOWN) {
    y+=1;
  } else if (direction == RIGHT) {
    x+=1;
  } else if (direction == LEFT){
    x-=1;
  }


  int idx = 0;

  while (true) {
    if (fakeSnakeCoords[idx][0] == -1){
      break;
    }
    idx++;
  }

  for (int i = idx;i>0;i--){
    fakeSnakeCoords[i][0] = fakeSnakeCoords[i-1][0];
    fakeSnakeCoords[i][1] = fakeSnakeCoords[i-1][1];
  }

  fakeSnakeCoords[0][0] = x;
  fakeSnakeCoords[0][1] = y;

  lc.setLed(0, fakeSnakeCoords[idx][0], fakeSnakeCoords[idx][1], false);

  fakeSnakeCoords[idx][0] = -1;
  fakeSnakeCoords[idx][1] = -1;
  lc.setLed(0, x+1, y, true);

}
void createSnake() {
  direction = DOWN;

    // setup snake 
  for(int i = 0;i<64;i++){
    snakeCoords[i][0] = -1;
    snakeCoords[i][1] = -1;
  }

  
  for(int i = 0;i<BODY_PARTS;i++){
    snakeCoords[i][0] = 0;
    snakeCoords[i][1] = 0;
  }

  for (int i = 0;i<64;i++){
    if (snakeCoords[i][0] != -1){
      lc.setLed(0, snakeCoords[i][0], snakeCoords[i][1],true);
    }
  }
}

void setImagesFalse(){
  playButtonDrawn = false;
  settingsKeyDrawn = false;
  aboutDrawn = false;
  leaderboardDrawn = false;
  fullMatrixDrawn = false;
}
// State,display related

void game() {
  control_joystick();

  if(!foodIn){
    createFood();
    foodIn = true;

    int idx = 0;

    while (true) {
      if (snakeCoords[idx][0] == -1){
        break;
      }
      if(snakeCoords[idx][0] == foodCoords[0] && snakeCoords[idx][1] == foodCoords[1]) {
        removeFood();
        lc.setLed(0, snakeCoords[idx][0], snakeCoords[idx][1], true);
        break;
      }
      idx++;
    }

  }

  reading = digitalRead(pinSW);
  if(reading != lastSwState) {
    lastDebounceTime = millis();
  }

  if(millis() - lastDebounceTime > buttonPressInterval){
    if(reading!=swState) {
      swState = reading;
      if(!swState){
        setImagesFalse();
        clearMatrix();
        foodIn = false;
        menuOption = lastMenuOption;
        mainState = MENU;
      }
    }
  }

  lastSwState = reading;


  if(millis() - timePassed > SPEED){
    timePassed = millis();
    next_turn();
  }
}

void controlNameInsertion(){
  if (joyMoved == false) {
    if(yValue < minThreshold){
      if(letterIndex!=0){
        letterIndex-=1;
      }
    } 
    if(yValue > maxThreshold){
      if(letterIndex!=2){
        letterIndex+=1;
      }
    }

    if(xValue < minThreshold){
      if(insertedLetters[letterIndex]!=0){
        insertedLetters[letterIndex]-=1;
      }
    } 
    if(xValue > maxThreshold){
      if(insertedLetters[letterIndex]!=26){
        insertedLetters[letterIndex]+=1;
      }
    }
    

    joyMoved = true;
  }

  if (xValue > minThreshold && xValue < maxThreshold && yValue > minThreshold && yValue < maxThreshold) {
    joyMoved = false;
  }
}

int blinkInterval = 500;
unsigned long blinkTime = 0;
bool blinkState = false;

bool changeState(bool blinkState){
    if (blinkState == true)
      return false;
    else
      return true;
}

char blinkLetter(){
  char characterOnScreen = alphabet[insertedLetters[letterIndex]];
  lcd.setCursor(7+letterIndex,1);
  
  if(millis() - blinkTime >= blinkInterval){
    blinkTime = millis();
    blinkState = changeState(blinkState);
  }

  if(blinkState){
    return characterOnScreen;
  }
  return byte(0);
}

bool xDrawn = false;
void over() {
  controlNameInsertion();
  lcd.setCursor(0,0);
  lcd.print("SCORE:");
  lcd.setCursor(7,0);
  lcd.print(lastScore);

  lcd.setCursor(0,1);
  lcd.print("NAME:");
  lcd.setCursor(7,1);
  for(int i = 0;i<3;i++){
    if(i == letterIndex){
      lcd.print(blinkLetter());
    } else {
      lcd.write(alphabet[insertedLetters[i]]);
    }
  }

  byte X[] = {B00000000,B01000010,B00100100,B00011000,B00011000,B00100100,B01000010,B00000000};

  if(!xDrawn){
    snakeScore = 0;
    clearMatrix();

    for (int i = 0;i<matrixSize;i++){
      lc.setRow(0,i,X[i]);
    }

  }

  reading = digitalRead(pinSW);
  if(reading != lastSwState) {
    lastDebounceTime = millis();
  }

  if(millis() - lastDebounceTime > buttonPressInterval){
    if(reading!=swState) {
      swState = reading;
      if(!swState){
        lcd.clear();
        clearMatrix();
        xDrawn = false;
        mainState = MENU;
        menuOption = 1;
        playerName[0] = alphabet[insertedLetters[0]];
        playerName[1] = alphabet[insertedLetters[1]];
        playerName[2] = alphabet[insertedLetters[2]];
        updateLeaderboard();
      }
    }
  }

  lastSwState = reading;
}

void updateLeaderboard(){
  highscore score1,score2,score3;
  EEPROM.get(scoreAddress1,score1);
  EEPROM.get(scoreAddress2,score2);
  EEPROM.get(scoreAddress3,score3);

  if(lastScore > score1.score){
    highscore newScore;
    strcpy(newScore.name, playerName);
    newScore.score = lastScore;

    EEPROM.put(scoreAddress1,newScore);
    EEPROM.put(scoreAddress2,score1);
    EEPROM.put(scoreAddress3,score2);
  }

  else if(lastScore > score2.score){
    highscore newScore;
    strcpy(newScore.name, playerName);
    newScore.score = lastScore;

    EEPROM.put(scoreAddress2,newScore);
    EEPROM.put(scoreAddress3,score2);
  }

  else if(lastScore > score2.score){
    highscore newScore;
    strcpy(newScore.name, playerName);
    newScore.score = lastScore;

    EEPROM.put(scoreAddress3,newScore);
  }

}

// ---------------------------------- MENU PART --------------------------------------------- //
String githubLink = "https://github.com/JohnPs1na";

int linkStart = 0;
int linkEnd = 0;
int scrollCursor = 16;
const int linkScrollSpeed = 300;


void controlLcdJoystick() {

  if (joyMoved == false) {
    if(xValue < minThreshold){
      if (menuOption!=4){
        menuOption+=1;
        clearMatrix();
      }
    } 
    if(xValue > maxThreshold){
      if (menuOption!=1){
        menuOption-=1;
        clearMatrix();
      }
    }

    joyMoved = true;
  }

  if (xValue > minThreshold && xValue < maxThreshold && yValue > minThreshold && yValue < maxThreshold) {
    joyMoved = false;
  }

  reading = digitalRead(pinSW);
  if(reading != lastSwState) {
    lastDebounceTime = millis();
  }

  if(millis() - lastDebounceTime > buttonPressInterval){
    if(reading!=swState) {
      swState = reading;
      if(!swState){
        if(menuOption == 1){
          lastMenuOption = menuOption;
          menuOption = 6969420;
          lcd.setCursor(0,0);
          lcd.print("SCORE:            ");
          lcd.setCursor(0,1);
          lcd.print("                 ");
          mainState = GAME;
          clearMatrix();
          createSnake();

        } else if (menuOption == 2) {
          for (int i = 0;i<matrixSize;i++){
            for (int j = 0;j<matrixSize;j++){
              lc.setLed(0,i,j,1);
            }
          }  
          lastMenuOption = menuOption;
          menuOption = 6969420;
          mainState = SETTINGS;
        } else if (menuOption == 3) {

          lcd.clear();
          leaderboardDrawn = false;
          lastMenuOption = menuOption;
          menuOption = 6969420;
          mainState = LEADERBOARD;

        } else if (menuOption == 4) {
          aboutDrawn = false;
          lastMenuOption = menuOption;
          menuOption = ABOUT;
        }
      }
    }
  }

  lastSwState = reading;
}

int settingsOption = 1;
int lastSettingsOption = 1;


void controlSettingsJoystick() {

  if(settingsOption > 3){
    return;
  }

  if (joyMoved == false) {
    if(xValue < minThreshold){
      if (settingsOption!=3){
        settingsOption+=1;
        clearMatrix();
      }
    } 
    if(xValue > maxThreshold){
      if (settingsOption!=1){
        fullMatrixDrawn = false;
        settingsOption-=1;
        clearMatrix();
      }
    }
    if(yValue < minThreshold) {
      mainState = MENU;
      clearMatrix();
      setImagesFalse();
      menuOption = lastMenuOption;
    }

    joyMoved = true;
  }

  if (xValue > minThreshold && xValue < maxThreshold && yValue > minThreshold && yValue < maxThreshold) {
    joyMoved = false;
  }

  reading = digitalRead(pinSW);
  if(reading != lastSwState) {
    lastDebounceTime = millis();
  }

  if(millis() - lastDebounceTime > buttonPressInterval){
    if(reading!=swState) {
      swState = reading;
      if(!swState){
        if(settingsOption == 1){
          lastSettingsOption = settingsOption;
          settingsOption = MATRIX_BRIGHTNESS;
        }
        if(settingsOption == 2){
          lastSettingsOption = settingsOption;
          settingsOption = DISPLAY_CONTRAST;
        }
        if(settingsOption == 3){
          lastSettingsOption = settingsOption;
          settingsOption = DIFFICULTY;
          createFakeSnake();
        }
      }
    }
  }

  lastSwState = reading;
}

void controlMatrix() {

  if (joyMoved == false) {
    if(yValue < minThreshold){
      if(settings[MATRIX_SETTINGS]!=1){
        settings[MATRIX_SETTINGS]-=1;
        lc.setIntensity(0,matrixBrightness*settings[MATRIX_SETTINGS]);
      }
    } 
    if(yValue > maxThreshold){
      if(settings[MATRIX_SETTINGS]!=3){
        settings[MATRIX_SETTINGS]+=1;
        lc.setIntensity(0,matrixBrightness*settings[MATRIX_SETTINGS]);
      }
    }

    joyMoved = true;
  }

  if (xValue > minThreshold && xValue < maxThreshold && yValue > minThreshold && yValue < maxThreshold) {
    joyMoved = false;
  }

  reading = digitalRead(pinSW);
  if(reading != lastSwState) {
    lastDebounceTime = millis();
  }

  if(millis() - lastDebounceTime > buttonPressInterval){
    if(reading!=swState) {
      swState = reading;
      if(!swState){
        settingsOption = 1;
      }
    }
  }

  lastSwState = reading;
}

void controlDisplay() {

  if (joyMoved == false) {
    if(yValue < minThreshold){
      if(settings[DISPLAY_SETTINGS]!=1){
        settings[DISPLAY_SETTINGS]-=1;
        analogWrite(lcdContrast,contrastValue*settings[DISPLAY_SETTINGS]);
      }
    } 
    if(yValue > maxThreshold){
      if(settings[DISPLAY_SETTINGS]!=3){
        settings[DISPLAY_SETTINGS]+=1;
        analogWrite(lcdContrast,contrastValue*settings[DISPLAY_SETTINGS]);
      }
    }

    joyMoved = true;
  }

  if (xValue > minThreshold && xValue < maxThreshold && yValue > minThreshold && yValue < maxThreshold) {
    joyMoved = false;
  }

  reading = digitalRead(pinSW);
  if(reading != lastSwState) {
    lastDebounceTime = millis();
  }

  if(millis() - lastDebounceTime > buttonPressInterval){
    if(reading!=swState) {
      swState = reading;
      if(!swState){
        settingsOption = 2;
      }
    }
  }

  lastSwState = reading;
}

void controlDifficulty() {

  if (joyMoved == false) {
    if(yValue < minThreshold){
      if(settings[DIFFICULTY_SETTINGS]!=1){
        settings[DIFFICULTY_SETTINGS]-=1;
        SPEED = diffSpeed/settings[DIFFICULTY_SETTINGS];
      }
    } 
    if(yValue > maxThreshold){
      if(settings[DIFFICULTY_SETTINGS]!=3){
        settings[DIFFICULTY_SETTINGS]+=1;
        SPEED = diffSpeed/settings[DIFFICULTY_SETTINGS];
      }
    }

    joyMoved = true;
  }

  if (xValue > minThreshold && xValue < maxThreshold && yValue > minThreshold && yValue < maxThreshold) {
    joyMoved = false;
  }

  reading = digitalRead(pinSW);
  if(reading != lastSwState) {
    lastDebounceTime = millis();
  }

  if(millis() - lastDebounceTime > buttonPressInterval){
    if(reading!=swState) {
      swState = reading;
      if(!swState){
        clearMatrix();
        settingsOption = 3;
      }
    }
  }

  lastSwState = reading;
}

void scrollText(){
    lcd.clear();
    if(linkStart == 0 && scrollCursor > 0){
      scrollCursor --;
      linkEnd++;
    } else if(linkStart == linkEnd) {
      linkStart = linkEnd = 0;
      scrollCursor = 16;
    } else if (linkEnd == githubLink.length() && scrollCursor == 0){
      linkStart++;
    } else {
      linkStart++;
      linkEnd++;
    }
}

bool menuGoBack(){

  bool triggered = false;
  if (joyMoved == false) {
    if(yValue < minThreshold){
      menuOption = lastMenuOption;
      setImagesFalse();
      triggered = true;
    }
    joyMoved = true;
  }

  if (yValue > minThreshold && yValue < maxThreshold && xValue > minThreshold && xValue < maxThreshold) {
    joyMoved = false;
  }

  return triggered;
}

bool settingsGoBack(){

  bool triggered = false;
  if (joyMoved == false) {
    if(yValue < minThreshold){
      settingsOption = lastSettingsOption;
      setImagesFalse();
      triggered = true;
    }
    joyMoved = true;
  }

  if (yValue > minThreshold && yValue < maxThreshold && xValue > minThreshold && xValue < maxThreshold) {
    joyMoved = false;
  }

  return triggered;
}


void lcdMenu(){
  switch (menuOption) {
    case 1:
      lcd.setCursor(0,0);
      lcd.print("Sneik                 ");
      lcd.setCursor(0,1);
      lcd.print("Play Game             ");
      lcd.setCursor(15,1);
      lcd.write("<");

      if(!playButtonDrawn) {
        setImagesFalse();
        clearMatrix();
        for (int i = 0;i<matrixSize;i++){
          lc.setRow(0,i,playButton[i]);
        }
        playButtonDrawn = true;
      }

      break;
    case 2:
      lcd.setCursor(0,0);
      lcd.print("Play Game            ");
      lcd.setCursor(0,1);
      lcd.print("Settings             ");
      lcd.setCursor(15,1);
      lcd.write("<");
      if(!settingsKeyDrawn) {
        setImagesFalse();
        clearMatrix();

        for (int i = 0;i<matrixSize;i++){
          lc.setRow(0,i,settingsKey[i]);
        }

        settingsKeyDrawn = true;
      }
      break;    

    case 3:
      lcd.setCursor(0,0);
      lcd.print("Settings            ");
      lcd.setCursor(0,1);
      lcd.print("Leaderboard         ");
      lcd.setCursor(15,1);
      lcd.write("<");
      if(!leaderboardDrawn) {
        setImagesFalse();
        clearMatrix();

        for (int i = 0;i<matrixSize;i++){
          lc.setRow(0,i,leaderboard[i]);
        }

        leaderboardDrawn = true;
      }
      break;
    case 4:
      lcd.setCursor(0,0);
      lcd.print("Leaderboard         ");
      lcd.setCursor(0,1);
      lcd.print("About              ");
      lcd.setCursor(15,1);
      lcd.write("<");
      if(!aboutDrawn) {
        setImagesFalse();
        clearMatrix();

        for (int i = 0;i<matrixSize;i++){
          lc.setRow(0,i,about[i]);
        }

        aboutDrawn = true;
      }
      break;
    case ABOUT:
      lcd.setCursor(0,0);
      lcd.print("Hadirca Dionisie");
      lcd.setCursor(scrollCursor,1);
      lcd.print(githubLink.substring(linkStart,linkEnd));
      if(millis() - timePassed > linkScrollSpeed){
          timePassed = millis();
          scrollText();
      }
      if(menuGoBack()){
        linkStart = 0;
        linkEnd = 0;
        scrollCursor = 16;
      };
      break;
  }
}
int leaderboardRow = 1;

void controlLeaderboardJoystick(){
  if (joyMoved == false) {
    if(xValue < minThreshold){
      if (leaderboardRow!=3){
        leaderboardRow+=1;
        lcd.clear();
      }
    } 
    if(xValue > maxThreshold){
      if (leaderboardRow!=1){
        leaderboardRow-=1;
        lcd.clear();
      }
    }
    if(yValue < minThreshold) {
      menuOption = lastMenuOption;
      mainState = MENU;
    }

    joyMoved = true;
  }

  if (xValue > minThreshold && xValue < maxThreshold && yValue > minThreshold && yValue < maxThreshold) {
    joyMoved = false;
  }
}

void displayLeaderboard(){
  highscore score1,score2,score3;
  EEPROM.get(scoreAddress1,score1);
  EEPROM.get(scoreAddress2,score2);
  EEPROM.get(scoreAddress3,score3);

  switch(leaderboardRow){
    case 1:
      lcd.setCursor(0,0);
      lcd.print("    HIGHSCORE    ");
      lcd.setCursor(0,1);
      lcd.print("1. ");
      lcd.setCursor(4,1);
      lcd.print(score1.name);
      lcd.setCursor(13,1);
      lcd.print(score1.score);
      
      lcd.setCursor(15,1);
      lcd.write("<");
      break;
    case 2:
      lcd.setCursor(0,0);
      lcd.print("1. ");
      lcd.setCursor(4,0);
      lcd.print(score1.name);
      lcd.setCursor(13,0);
      lcd.print(score1.score);
      lcd.setCursor(0,1);
      lcd.print("2. ");
      lcd.setCursor(4,1);
      lcd.print(score2.name);
      lcd.setCursor(13,1);
      lcd.print(score2.score);
      
      lcd.setCursor(15,1);
      lcd.write("<");
      break;
    case 3:
      lcd.setCursor(0,0);
      lcd.print("2. ");
      lcd.setCursor(4,0);
      lcd.print(score2.name);
      lcd.setCursor(13,0);
      lcd.print(score2.score);
      lcd.setCursor(0,1);
      lcd.print("3. ");
      lcd.setCursor(4,1);
      lcd.print(score3.name);
      lcd.setCursor(13,1);
      lcd.print(score3.score);
      
      lcd.setCursor(15,1);
      lcd.write("<");
      break;
    
    
  }
}
void displaySettings(){
  switch (settingsOption) {
    case 1:
      lcd.setCursor(0,0);
      lcd.print("Matrix               ");
      lcd.setCursor(15,0);
      lcd.print("<");
      lcd.setCursor(0,1);
      lcd.print("Display               ");
      if(!fullMatrixDrawn) {
        setImagesFalse();
        clearMatrix();

        for (int i = 0;i<matrixSize;i++){
          lc.setRow(0,i,fullMatrix[i]);
        }

        fullMatrixDrawn = true;
      }
      break;
    case 2:
      lcd.setCursor(0,0);
      lcd.print("Display               ");
      lcd.setCursor(15,0);
      lcd.print("<");
      lcd.setCursor(0,1);
      lcd.print("Difficulty            ");
      break;
    case 3:
      lcd.setCursor(0,0);
      lcd.print("Difficulty            ");
      lcd.setCursor(15,0);
      lcd.print("<");
      lcd.setCursor(0,1);
      lcd.print("                       ");
      break;
    case MATRIX_BRIGHTNESS:
      controlMatrix();
      lcd.setCursor(0,0);
      lcd.print("Matrix");
      lcd.setCursor(11,0);
      for(int i = 0;i<settings[MATRIX_SETTINGS];i++){
        lcd.write(byte(0));
      }
      for(int i = 11+settings[MATRIX_SETTINGS];i<15;i++){
        lcd.print(" ");
      }
      break;
    case DISPLAY_CONTRAST:
      controlDisplay();
      lcd.setCursor(0,0);
      lcd.print("Display");
      lcd.setCursor(11,0);
      for(int i = 0;i<settings[DISPLAY_SETTINGS];i++){
        lcd.write(byte(0));
      }
      for(int i = 11+settings[DISPLAY_SETTINGS];i<15;i++){
        lcd.print(" ");
      }
      break;
    case DIFFICULTY:
      if(millis() - timePassed > SPEED){
        timePassed = millis();
        nextFakeTurn();
      }
      controlDifficulty();
      lcd.setCursor(0,0);
      lcd.print("Difficulty");
      lcd.setCursor(11,0);
      for(int i = 0;i<settings[DIFFICULTY_SETTINGS];i++){
        lcd.write(byte(0));
      }
      for(int i = 11+settings[DIFFICULTY_SETTINGS];i<15;i++){
        lcd.print(" ");
      }
      break;
  }
}
void setup() {
  // put your setup code here, to run once:
  lc.shutdown(0,false);
  lc.setIntensity(0,matrixBrightness*settings[MATRIX_SETTINGS]);
  lc.clearDisplay(0);
  pinMode(pinSW,INPUT_PULLUP);
  
  lcd.begin(16,2);
  lcd.createChar(0,squareLCD);

  analogWrite(lcdContrast,contrastValue*settings[DISPLAY_SETTINGS]);

  Serial.begin(9600);
}


void loop() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  
  switch(mainState){
    case MENU:
      controlLcdJoystick();
      lcdMenu();
      break;
    case GAME:
      game();
      lcd.setCursor(0,0);
      lcd.print("SCORE:            ");
      lcd.setCursor(0,1);
      lcd.print(snakeScore);
      break;
    case OVER:
      over();
      if(menuGoBack()){
        mainState = MENU;
        clearMatrix();
        setImagesFalse();
        menuOption = lastMenuOption;
      }
      break;
    case SETTINGS:
      controlSettingsJoystick();
      displaySettings();
      break;
    case LEADERBOARD:
      controlLeaderboardJoystick();
      displayLeaderboard();
      break;


  }
}

