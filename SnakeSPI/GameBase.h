#ifndef GameBase_h
#define GameBase_h

#include <Arduino.h>

#define NUM_MATRICES 1
#define MATRIX_SIZE 8
#define MAX_SNAKE_LENGTH NUM_MATRICES*MATRIX_SIZE*MATRIX_SIZE

typedef enum{up,down,right,left} direction;

typedef struct Dot Dot;

struct Dot {
  int row;
  int column; 
};

class Snake {
  private:
    int length;
    direction snakeDirection;
    Dot body[MAX_SNAKE_LENGTH];
  public:
    Snake();
    void generateSnake();
    void setDirection(direction newDirection);
    direction returnDirection();
    void setLength(int newLength);
    int returnLength();
    void setRowAtIndex(int index, int row);
    int returnRowAtIndex(int index);
    void setColumnAtIndex(int index, int column);
    int returnColumnAtIndex(int index);
};

#endif