#include "SnakeGame.h" 

SnakeGame::SnakeGame(int CS, int Xpin, int Ypin) 
  :snakeObject()
{
  pinMode(CS,OUTPUT);
  pinMode(Xpin,INPUT);
  pinMode(Ypin,INPUT);
  _CS = CS;
  _Xpin = Xpin;
  _Ypin = Ypin;
  gameState = preGame;
  score = 0;
  oldScore = score;
  for(int i = 0; i < NUM_MATRICES * MATRIX_SIZE; i++)
    frame[i] = 0;
  for(int i = 0; i < 4; i++)
    tempArray[i] = {'\0', 0};
}
int SnakeGame::returnCS() {
  return _CS;
}
void SnakeGame::gameStart() { 
  score = 0; // starting score value 
  snakeObject.generateSnake(); // function to randomly generate snake on the screen
  SnakeGame::generateFood(); // function to randomly generate food on the screen
}
int SnakeGame::readJoystickX() {
  int xValue = analogRead(_Xpin);
  return xValue;
}
int SnakeGame::readJoystickY() {
  int yValue = analogRead(_Ypin);
  return yValue;
}
void SnakeGame::checkSnakeDirection() { // read joystick values and based on them update snake direction
  /*
              0
              |
              |
              |
  0-----------.--------->1023
              |
              |
              |
            1023
  */
  int xVal = SnakeGame::readJoystickX(); // value of x-axis increases when going right, decreases when going left
  int yVal = SnakeGame::readJoystickY(); // value of y-axis increases when going down, decreases when going up
  switch(snakeObject.returnDirection()) { 
    case right: 
      if (yVal < 455) { 
        snakeObject.setDirection(up);
      } else if (yVal > 555) { 
        snakeObject.setDirection(down);
      }
    break;
    case left: 
      if (yVal < 455) {
        snakeObject.setDirection(up);
      } else if (yVal > 555) {
        snakeObject.setDirection(down);
      }
    break;
    case up: 
      if (xVal < 455) {
        snakeObject.setDirection(left);
      } else if (xVal > 555) {
        snakeObject.setDirection(right);
      }
    break;
    case down:
      if (xVal < 455) {
        snakeObject.setDirection(left);
      } else if (xVal > 555) {
        snakeObject.setDirection(right);
      }
    break;
  }
}
void SnakeGame::updateFrame() { 
  SnakeGame::resetFrame(); // set all rows of the frame to 0
  SnakeGame::newHeadCoordinates(); // generate new head coordinates (in the next frame)
  SnakeGame::checkBorders(); // if new coordinates exceed the maximum value of borders, set to the opposite side value (for example 8 to 0)
  SnakeGame::hitCheck(); // if the snake hits itself, set game state to gameover
  SnakeGame::updateSnake(); // updates snake body coordinates
  SnakeGame::showSnake(); // draw snake on the frame
  SnakeGame::showFood(); // draw food on the frame
}
void SnakeGame::newHeadCoordinates() {
  switch(snakeObject.returnDirection()) {
    case up:
      newHead.row = snakeObject.returnRowAtIndex(snakeObject.returnLength() - 1) - 1;
      newHead.column = snakeObject.returnColumnAtIndex(snakeObject.returnLength() - 1);
    break;
    case down:
      newHead.row = snakeObject.returnRowAtIndex(snakeObject.returnLength() - 1) + 1;
      newHead.column = snakeObject.returnColumnAtIndex(snakeObject.returnLength() - 1);
    break;
    case right:
      newHead.row = snakeObject.returnRowAtIndex(snakeObject.returnLength() - 1);
      newHead.column = snakeObject.returnColumnAtIndex(snakeObject.returnLength() - 1) + 1;
    break;
    case left:
      newHead.row = snakeObject.returnRowAtIndex(snakeObject.returnLength() - 1);
      newHead.column = snakeObject.returnColumnAtIndex(snakeObject.returnLength() - 1) - 1;
    break;
  }
}
void SnakeGame::checkBorders() {
  if (newHead.row == MATRIX_SIZE) {
    newHead.row = 0;
  } else if (newHead.row == -1) {
    newHead.row = MATRIX_SIZE - 1;
  }
  if (newHead.column == MATRIX_SIZE * NUM_MATRICES) {
    newHead.column = 0;
  } else if (newHead.column == -1) {
    newHead.column = MATRIX_SIZE * NUM_MATRICES - 1;
  }
}
void SnakeGame::hitCheck() {
  for (int j = 0; j < snakeObject.returnLength(); j++) {
    if ((snakeObject.returnRowAtIndex(j) == newHead.row && snakeObject.returnColumnAtIndex(j) == newHead.column)) {
      gameState = gameOver;
    }
  }
}
void SnakeGame::updateSnake() {
  if (newHead.row == food.row && newHead.column == food.column) { // if food is eaten, increase score, increase length of the snake 
    score++;
    snakeObject.setLength(snakeObject.returnLength() + 1);
    if(SnakeGame::maxLengthCheck()) // if max length is reached, set game state to gameover
      gameState = gameOver;
    else // if not, generate food
      SnakeGame::generateFood();
  } 
  else {
    SnakeGame::shiftSnakeBodyLeft(); // coordinates of the snake body in the next frame
  }
  SnakeGame::setNewSnakeHead(); // update head coordinates
}
void SnakeGame::shiftSnakeBodyLeft() { 
  for (int j = 1; j < snakeObject.returnLength(); j++) {
    snakeObject.setRowAtIndex(j - 1, snakeObject.returnRowAtIndex(j));
    snakeObject.setColumnAtIndex(j - 1, snakeObject.returnColumnAtIndex(j));
  }
}
void SnakeGame::setNewSnakeHead() {
  snakeObject.setRowAtIndex(snakeObject.returnLength() - 1, newHead.row);
  snakeObject.setColumnAtIndex(snakeObject.returnLength() - 1, newHead.column);
}
void SnakeGame::showSnake() { // each row's first bit is set to '1' and then moved to the correct position
  for (int j = 0; j < snakeObject.returnLength(); j++) { // there are MATRIX_SIZE rows, and NUM_MATRICES*MATRIX_SIZE columns
    int row = snakeObject.returnRowAtIndex(j);
    int column = snakeObject.returnColumnAtIndex(j);
    int matrixIndex = column / MATRIX_SIZE; // determine in which matrix is the snake part located
    int localColumn = column % MATRIX_SIZE; // determine in which column of that matrix is snake part located
    int localRow = row + matrixIndex * MATRIX_SIZE; // determine the row of the local matrix 
    frame[localRow] |= 128 >> localColumn; // '1' at the start of the row of the local matrix is moved to the correct column
  }
}
void SnakeGame::showFood() { // analog to showSnake
  int matrixIndex = food.column / MATRIX_SIZE;
  int localColumn = food.column % MATRIX_SIZE;
  int localRow = food.row + matrixIndex * MATRIX_SIZE;
  frame[localRow] |= 128 >> localColumn;
}
void SnakeGame::resetFrame() { // every row is set to zero
  for (int i = 0; i < MATRIX_SIZE * NUM_MATRICES; i++) {
    frame[i] = 0;
  }
}
byte SnakeGame::returnFrameRowAtIndex(int index) {
  return frame[index];
}
State SnakeGame::checkGameState() {
  return gameState;
}
void SnakeGame::setGameState(State newState) {
  gameState = newState;
}
int SnakeGame::returnScore() {
  return score;
}
bool SnakeGame::foodIsGeneratedOnSnakeBody() { // if food is generated at some part of snake's body return true
	bool condition = false;
	for(int i = 0; i < snakeObject.returnLength(); i++) 
		if((snakeObject.returnRowAtIndex(i) == food.row) && (snakeObject.returnColumnAtIndex(i) == food.column)) 
			condition = true;
	return condition;
}
void SnakeGame::generateFood() { // generate food coordinates so that it is not generated on the snake's body
  do {
    food = {(int)random(0, MATRIX_SIZE), (int)random(0, MATRIX_SIZE * NUM_MATRICES)};
  } while (foodIsGeneratedOnSnakeBody());
}
bool SnakeGame::maxLengthCheck() {
  if(snakeObject.returnLength() == MAX_SNAKE_LENGTH)
    return true;
  else 
    return false;
}
void SnakeGame::incrementUpdateRateIfNeeded(float &updateRate) { 
  if(((score == MAX_SNAKE_LENGTH / 6) || (score == MAX_SNAKE_LENGTH / 5) || (score == MAX_SNAKE_LENGTH / 4) || (score == MAX_SNAKE_LENGTH / 3) || (score == MAX_SNAKE_LENGTH / 2)) && (score != oldScore)) 
    updateRate++;
  oldScore = score;
}
void SnakeGame::EEPROMsetStartScoresCount() {
  int startScoresCount;
  EEPROM.get(0,startScoresCount); // read from '0' address 
  if(startScoresCount == 0xFFFF) { // at start all memory units are set to '1', if that is true, set to 0
    startScoresCount = 0x0;
    EEPROM.put(0, startScoresCount); // update data at '0' address to 0
  }
}
int SnakeGame::EEPROMreadScoresCount() { // returns current player count 
  int scoresCount; 
  EEPROM.get(0,scoresCount);
  return scoresCount;
}
void SnakeGame::EEPROMupdateScoresCount() { // function to increment scores count in memory, only when it is lower than 3
  int scoresCount = SnakeGame::EEPROMreadScoresCount();
  if(scoresCount < 3) {
    scoresCount++;
  }
  EEPROM.put(0,scoresCount);
}
void SnakeGame::resetTempArray() { // set all player names in temp array to null terminator and scores to 0
  for(int i = 0; i < 4; i++) {
    tempArray[i].name[0] = '\0';
    tempArray[i].score = 0;
  }
}
int SnakeGame::numberOfPlayersInTempArray() {
  int playerCount = 0;
  while(tempArray[playerCount].name[0] != '\0') {
    playerCount++;
    if(playerCount == 4) 
      break;
  }
  return playerCount;
}
void SnakeGame::sortTempArray() { // by sorting temporary array, find the correct place for current player
  int i = 0, j = 0, k = 0;
  int playerCount = SnakeGame::numberOfPlayersInTempArray();
  for(i = 0; i < playerCount - 1; i++) {
    int max = i;
    for(j = i + 1; j < playerCount; j++) {
      if(tempArray[j].score > tempArray[max].score)
        max = j;
    }
    if(max != i) {
      Player temp;
      for(k = 0; k < 4; k++) 
        temp.name[k] = tempArray[i].name[k];
      temp.score = tempArray[i].score;
      for(k = 0; k < 4; k++) 
        tempArray[i].name[k] = tempArray[max].name[k];
      tempArray[i].score = tempArray[max].score;
      for(k = 0; k < 4; k++) 
        tempArray[max].name[k] = temp.name[k];
      tempArray[max].score = temp.score;
    }
  }
}
void SnakeGame::putPlayerInfoInLastPlaceOfTempArray(char *userName, int currentScore) {
  int lastIndex = SnakeGame::numberOfPlayersInTempArray();
  for(int i = 0; i < 4; i++) {
    tempArray[lastIndex].name[i] = userName[i];
  }
  tempArray[lastIndex].score = currentScore;
}
void SnakeGame::EEPROMputPlayerInfoInMemory(char *userName, int currentScore) {
  SnakeGame::EEPROMreadPlayersInfoFromMemoryAndStoreInTempArray();
  SnakeGame::putPlayerInfoInLastPlaceOfTempArray(userName, currentScore);
  SnakeGame::sortTempArray(); // sort the temporary array, where first three players are read from memory and fourth is the current player
  SnakeGame::EEPROMwriteBestScoresFromTempArrayInMemory();
  SnakeGame::EEPROMupdateScoresCount();
  SnakeGame::resetTempArray();
}
void SnakeGame::EEPROMreadPlayersInfoFromMemoryAndStoreInTempArray() {
  int scoresCount = SnakeGame::EEPROMreadScoresCount();
  int eeAddress = sizeof(scoresCount); // moves to next memory address, after integer scores count
  for(int i = 0; i < scoresCount; i++) {
    EEPROM.get(eeAddress,tempArray[i]); // store player info at each index of temp array
    eeAddress += sizeof(Player); // next address is shifted by the size of player struct
  }
}
int SnakeGame::EEPROMreadLastPlaceScoreFromMemory() {
  Player tempPlayer = {'\0', 0}; // empty temporary player
  int scoresCount = SnakeGame::EEPROMreadScoresCount();
  int eeAddress = sizeof(scoresCount);
  if(scoresCount == 3) { // based on number of players, determine eeprom address of last place player
    eeAddress += 2 * sizeof(Player);
  } else if(scoresCount == 2) {
    eeAddress += sizeof(Player);
  } else if(scoresCount == 0) {
    return tempPlayer.score;
  }
  EEPROM.get(eeAddress,tempPlayer); // store last place score in temporary player
  return tempPlayer.score;
}
void SnakeGame::EEPROMwriteBestScoresFromTempArrayInMemory() { // store first three scores from sorted temporary array in memory
  int eeAddress = sizeof(int);
  int playerCount = SnakeGame::numberOfPlayersInTempArray();
  if(playerCount > 3)
    playerCount = 3;
  for(int i = 0; i < playerCount; i++) {
    EEPROM.put(eeAddress,tempArray[i]);
    eeAddress += sizeof(Player);
  }
}