#include <SPI.h>
#include <LiquidCrystal.h>
#include "SnakeGame.h"

#define taster1Pin 2
#define taster2Pin 3

SnakeGame sg(10, A0, A1);
LiquidCrystal lcd(4, 5, 6, 7, 8, 9);

String firstRowAtStart = "---Welcome-to---";
String secondRowAtStart = "---Snake-Game---";

String firstRowPreGame = "1 - start/quit";
String secondRowPreGame = "2 - show scores";

String firstRowInGame = "Your score is:";

String firstRowGameOver = "Final score:";

String firstRowShowScores1 = "----Current-----";
String secondRowShowScores1 = "---Scoreboard---";

String firstRowShowScores2 = "------1st-------";
String secondRowShowScores2 = "--2nd------3rd--";

String firstRowNoScoresInMemory = "--No-Scores-In--";
String secondRowNoScoresInMemory = "-----Memory-----";

String firstRowUserInput = "Your 3-character";
String secondRowUserInput = "name: ";

String firstRowScorePutInMemory = "---Your-score---";
String secondRowScorePutInMemory = "----is-saved----";

volatile bool startQuitFlag = false;
volatile bool showScoresFlag = false;

float oldTime = 0;
float timer = 0;
float updateRate = 3;
int oldScore = 0;

void setup() {

  Serial.begin(9600);
  SPI.begin();

  digitalWrite(sg.returnCS(), HIGH);
  sendCommand(0x0C, 0x01); // Shutdown register, normal operation
  sendCommand(0x0F, 0x00); // Display test register, normal operation
  sendCommand(0x09, 0x00); // Decode mode register, no decode
  sendCommand(0x0B, 0x07); // Scan limit register, display digits 0-7
  sendCommand(0x0A, 0x08); // Intensity register, medium intensity

  pinMode(taster1Pin,INPUT);
  pinMode(taster2Pin,INPUT);
  attachInterrupt(digitalPinToInterrupt(taster1Pin), taster1ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(taster2Pin), taster2ISR, RISING);

  lcd.begin(16,2);
  lcdShowText(firstRowAtStart,secondRowAtStart);
  delay(3000);

  sg.EEPROMsetStartScoresCount();

}

void loop() {

  float deltaTime = calculateDeltaTime();
  timer += deltaTime;

  int currentScore = sg.returnScore();
  String secondRowInGame = String(currentScore);
  String secondRowGameOver = String(currentScore);

  sg.incrementUpdateRateIfNeeded(updateRate);
  State currentState = sg.checkGameState();

  switch(currentState) {
    case preGame:

      updateRate = 3;
      sg.resetFrame();
      showFrame();
      if(timer > 1000) {
        lcdShowText(firstRowPreGame,secondRowPreGame);
        timer = 0;
      }

      if(showScoresFlag) {
        noInterrupts();
        int scoresCount = sg.EEPROMreadScoresCount();
        interrupts();
        if(scoresCount == 0x0) {
          lcdShowText(firstRowNoScoresInMemory,secondRowNoScoresInMemory);
          delay(3000);
        }
        else {
          lcdShowText(firstRowShowScores1,secondRowShowScores1);
          delay(3000);
          lcdShowText(firstRowShowScores2,secondRowShowScores2);
          delay(3000);
          noInterrupts();
          lcdShowScores(); 
          interrupts();
          delay(3000);
        }
        showScoresFlag = false;
      }

      if(startQuitFlag) { 
        sg.gameStart();
        sg.setGameState(inGame);
        startQuitFlag = false;
      }

    break;
    case inGame:

      sg.checkSnakeDirection();
      if(timer > 1000 / updateRate) {
        sg.updateFrame();
        lcdShowText(firstRowInGame,secondRowInGame);
        timer = 0;
      }
      showFrame();

      if(startQuitFlag) { 
        sg.setGameState(preGame);
        startQuitFlag = false;
      }

    break;
    case gameOver:

      sg.resetFrame();
      showFrame();
      lcdShowText(firstRowGameOver,secondRowGameOver);
      delay(3000);

      noInterrupts();
      int lastPlaceScore = sg.EEPROMreadLastPlaceScoreFromMemory();
      interrupts();
      if(currentScore > lastPlaceScore) {
        char userName[4];
        int charPositionInUserName = 0;
        secondRowUserInput = "name: ";
        lcdShowText(firstRowUserInput,secondRowUserInput);
        while(!Serial.available()) {} 
        while(true) { 
          if(Serial.available()) {
            char c = Serial.read();
            if(charPositionInUserName == 3) {
              userName[charPositionInUserName] = '\0';
              break;
            } else {
              userName[charPositionInUserName] = c;
              secondRowUserInput += c;
              lcdShowText(firstRowUserInput,secondRowUserInput);
              charPositionInUserName++;
            }
          }
        }
        while(Serial.available()) {
          Serial.read(); 
        }
        noInterrupts();
        sg.EEPROMputPlayerInfoInMemory(userName, currentScore);
        interrupts();
        lcdShowText(firstRowScorePutInMemory,secondRowScorePutInMemory);
        delay(3000);
      }
      sg.setGameState(preGame);

    break;
  }

}

void taster1ISR() {
  startQuitFlag = true;
}

void taster2ISR() {
  showScoresFlag = true;
}

float calculateDeltaTime() {
  float currentTime = millis();
  float dt = currentTime - oldTime;
  oldTime = currentTime;
  return dt;
}

void sendCommand(byte address, byte value) {
  int cs = sg.returnCS();
  digitalWrite(cs, LOW);
  for (int matrix = 0; matrix < NUM_MATRICES; matrix++) {
    SPI.transfer(address); 
    SPI.transfer(value); 
  }
  digitalWrite(cs, HIGH);
}

void showFrame() {
  int cs = sg.returnCS();
  for (int row = 0; row < MATRIX_SIZE; row++) {
    digitalWrite(cs, LOW);
    for (int matrix = 0; matrix < NUM_MATRICES; matrix++) {
      SPI.transfer(row + 1); 
      SPI.transfer(sg.returnFrameRowAtIndex(row + matrix * MATRIX_SIZE));
    }
    digitalWrite(cs, HIGH);
  }
}

void lcdShowText(String firstRowText, String secondRowText) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(firstRowText);
  lcd.setCursor(0,1);
  lcd.print(secondRowText);
}

void lcdShowScores() { 
  sg.EEPROMreadPlayersInfoFromMemoryAndStoreInTempArray();
  int scoresCount = sg.EEPROMreadScoresCount();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("----");
  if(sg.tempArray[0].score < 10 || sg.tempArray[0].score < 100)
    lcd.print("-");
  lcd.print(sg.tempArray[0].name);
  lcd.print(":");
  lcd.print(sg.tempArray[0].score);
  lcd.print("-----");
  if(sg.tempArray[0].score < 10)
    lcd.print("-");
  lcd.setCursor(0,1);
  if(scoresCount > 1) {
    if(sg.tempArray[1].score < 10)
      lcd.print("--");
    else if(sg.tempArray[1].score < 100)
      lcd.print("-");
    lcd.print(sg.tempArray[1].name);
    lcd.print(":");
    lcd.print(sg.tempArray[1].score);
    if(scoresCount > 2) {
      lcd.print("--");
      lcd.print(sg.tempArray[2].name);
      lcd.print(":");
      lcd.print(sg.tempArray[2].score);
      if(sg.tempArray[2].score < 10)
        lcd.print("--");
      else if(sg.tempArray[2].score < 100)
        lcd.print("-");
    } else {
      lcd.print("---------");
    }
  } else {
    lcd.print("----------------");
  }
  sg.resetTempArray();
}