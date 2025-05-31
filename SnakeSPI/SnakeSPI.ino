#include <SPI.h> 
#include <LiquidCrystal.h>
#include "SnakeGame.h"

#define taster1Pin 2 // first interrupt button
#define taster2Pin 3 // second interrupt button

SnakeGame sg(10, A0, A1); // constructor for the game, cs pin, x and y axis joystick pin
LiquidCrystal lcd(4, 5, 6, 7, 8, 9); 

// strings for lcd display
String firstRowAtStart = "---Welcome-to---";
String secondRowAtStart = "---Snake-Game---";

String firstRowPreGame = "YLW - start/quit";
String secondRowPreGame = "RED - show scores";

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

volatile bool startQuitFlag = false; // interrupt flags
volatile bool showScoresFlag = false;

float oldTime = 0; // variables for update rate
float timer = 0;
float updateRate = 3;
int oldScore = 0; 

void setup() {

  Serial.begin(9600);
  SPI.begin();

  digitalWrite(sg.returnCS(), HIGH); // when CS is high, no communication occurs
  sendCommand(0x0C, 0x01); // Shutdown register, normal operation
  sendCommand(0x0F, 0x00); // Display test register, normal operation
  sendCommand(0x09, 0x00); // Decode mode register, no decode
  sendCommand(0x0B, 0x07); // Scan limit register, display digits 0-7
  sendCommand(0x0A, 0x01); // Intensity register, medium intensity

  pinMode(taster1Pin,INPUT); 
  pinMode(taster2Pin,INPUT); 
  attachInterrupt(digitalPinToInterrupt(taster1Pin), taster1ISR, RISING); // pull-down resistors, so rising edge
  attachInterrupt(digitalPinToInterrupt(taster2Pin), taster2ISR, RISING);

  lcd.begin(16,2);
  lcdShowText(firstRowAtStart,secondRowAtStart); // start screen
  delay(3000);

  sg.EEPROMsetStartScoresCount(); // function to set the start scores count in first address od EEPROM

}

void loop() {

  float deltaTime = calculateDeltaTime(); // time needed for one program run 
  timer += deltaTime;

  int currentScore = sg.returnScore();
  String secondRowInGame = String(currentScore); // convert current score to string for second row of lcd
  String secondRowGameOver = String(currentScore); 

  sg.incrementUpdateRateIfNeeded(updateRate);
  State currentState = sg.checkGameState();

  switch(currentState) { // three game states
    case preGame:

      updateRate = 3; // starting update rate
      sg.resetFrame(); 
      showFrame();
      if(timer > 1000) { // refresh rate
        lcdShowText(firstRowPreGame,secondRowPreGame); // screen that shows button functions
        timer = 0;
      }

      if(showScoresFlag) {
        noInterrupts(); // disables interrupts while working with EEPROM, so that the memory is not corrupted
        int scoresCount = sg.EEPROMreadScoresCount();
        interrupts(); // when EEPROM function is done, enable interrupt
        if(scoresCount == 0x0) { 
          lcdShowText(firstRowNoScoresInMemory,secondRowNoScoresInMemory); // special screen when there are no scores in memory
          delay(3000);
        }
        else {
          lcdShowText(firstRowShowScores1,secondRowShowScores1);
          delay(3000);
          lcdShowText(firstRowShowScores2,secondRowShowScores2);
          delay(3000);
          noInterrupts();
          lcdShowScores(); // shows the screen with scores by reading info from EEPROM
          interrupts();
          delay(3000);
        }
        showScoresFlag = false; // reset the interrupt flag
      }

      if(startQuitFlag) { // if startquit button is pressed, start the game and update game state to ingame
        sg.gameStart();
        sg.setGameState(inGame);
        startQuitFlag = false;
      }

    break;
    case inGame:

      sg.checkSnakeDirection(); // update current snake direction
      if(timer > 1000 / updateRate) { // refresh rate
        sg.updateFrame(); // based on the current information, update the frame
        lcdShowText(firstRowInGame,secondRowInGame); // in game screen
        timer = 0; // reset the timer
      }
      showFrame(); // after it is updated, show the frame on matrices

      if(startQuitFlag) { // if the player presses the button in game, set game state to pregame
        sg.setGameState(preGame);
        startQuitFlag = false;
      }

    break;
    case gameOver:

      sg.resetFrame(); // no LEDs of matrices are lit in this game state
      showFrame(); 
      lcdShowText(firstRowGameOver,secondRowGameOver); // screen to show the final game score
      delay(3000);

      noInterrupts(); 
      int lastPlaceScore = sg.EEPROMreadLastPlaceScoreFromMemory(); // variable to store last place score from memory
      interrupts();
      if(currentScore > lastPlaceScore) { // only store current player info if it is better than last place score
        char userName[4];
        int charPositionInUserName = 0;
        secondRowUserInput = "name: "; // reset the second row after last user input
        lcdShowText(firstRowUserInput,secondRowUserInput);
        while(!Serial.available()) {} // while serial input buffer is empty, do nothing
        while(true) { 
          if(Serial.available()) {
            char c = Serial.read(); // char on input
            if(charPositionInUserName == 3) {
              userName[charPositionInUserName] = '\0';
              break; // after three characters are input, break from loop
            } else {
              userName[charPositionInUserName] = c;
              secondRowUserInput += c;
              lcdShowText(firstRowUserInput,secondRowUserInput);
              charPositionInUserName++; // increment position in name for next step
            }
          }
        }
        while(Serial.available()) { // empties serial input buffer
          Serial.read(); 
        }
        noInterrupts();
        sg.EEPROMputPlayerInfoInMemory(userName, currentScore); // put current player info in correct place in eeprom memory
        interrupts();
        lcdShowText(firstRowScorePutInMemory,secondRowScorePutInMemory); // screen to confirm that info is stored in memory
        delay(3000);
      }
      sg.setGameState(preGame); // when game is done, return to pregame state

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
  float currentTime = millis(); // current time from program start in miliseconds
  float dt = currentTime - oldTime; 
  oldTime = currentTime;
  return dt;
}

void sendCommand(byte address, byte value) { // send the same command to all matrices (used for register setting only)
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
  for (int row = 0; row < MATRIX_SIZE; row++) { // appropriate row data is sent to each matrix
    digitalWrite(cs, LOW);
    for (int matrix = 0; matrix < NUM_MATRICES; matrix++) {
      SPI.transfer(row + 1); // addresses are indexed 1 to 8, so +1 
      SPI.transfer(sg.returnFrameRowAtIndex(row + matrix * MATRIX_SIZE)); // in one transaction, row at same index is sent to each matrix
      // in order to reach the second matrix all other rows of the first matrix are skipped
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