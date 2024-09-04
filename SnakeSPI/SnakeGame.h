#ifndef SnakeGame_h
#define SnakeGame_h

#include <EEPROM.h>
#include "GameBase.h"

typedef struct Player Player;

struct Player {
  char name[4];
  int score;
};

typedef enum{preGame,inGame,gameOver} State;

class SnakeGame {
  private:
    int _CS;
    int _Xpin;
    int _Ypin;
    int score;
    int oldScore;
    byte frame[MATRIX_SIZE * NUM_MATRICES];
    Dot food;
    Dot newHead;
    Snake snakeObject;
    State gameState;
    void newHeadCoordinates();
    void checkBorders();
    void hitCheck();
    void updateSnake();
    void shiftSnakeBodyLeft();
    void setNewSnakeHead();
    void showSnake();
    void showFood();
    int readJoystickX();
    int readJoystickY();
    bool foodIsGeneratedOnSnakeBody();
    void generateFood();
    bool maxLengthCheck();
    int numberOfPlayersInTempArray();
    void sortTempArray();
    void putPlayerInfoInLastPlaceOfTempArray(char *userName, int currentScore);
    void EEPROMupdateScoresCount();
    void EEPROMwriteBestScoresFromTempArrayInMemory();
  public:
    SnakeGame(int CS, int Xpin, int Ypin);
    void gameStart();
    byte returnFrameRowAtIndex(int index);
    void checkSnakeDirection();
    void resetFrame();
    void updateFrame();
    int returnCS();
    void setGameState(State newState);
    State checkGameState();
    int returnScore();
    void incrementUpdateRateIfNeeded(float &updateRate);
    Player tempArray[4];
    void resetTempArray(); 
    void EEPROMputPlayerInfoInMemory(char *userName, int currentScore); 
    void EEPROMsetStartScoresCount(); 
    int EEPROMreadScoresCount(); 
    void EEPROMreadPlayersInfoFromMemoryAndStoreInTempArray(); 
    int EEPROMreadLastPlaceScoreFromMemory(); 
};

#endif