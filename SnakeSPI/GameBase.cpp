#include "GameBase.h"

Snake::Snake() {
  length = 0;
}
void Snake::generateSnake() {
  Snake::setLength(2);
  Snake::setRowAtIndex(1,(int)random(1, MATRIX_SIZE - 1)); 
  Snake::setColumnAtIndex(1,(int)random(1, MATRIX_SIZE * NUM_MATRICES - 1)); 
  Snake::setDirection((direction)random(0,4));
  switch(Snake::returnDirection()) {
    case up:
      Snake::setRowAtIndex(0,returnRowAtIndex(1) + 1); 
      Snake::setColumnAtIndex(0,returnColumnAtIndex(1));
    break;
    case down:
      Snake::setRowAtIndex(0,returnRowAtIndex(1) - 1); 
      Snake::setColumnAtIndex(0,returnColumnAtIndex(1));
    break;
    case right:
      Snake::setRowAtIndex(0,returnRowAtIndex(1)); 
      Snake::setColumnAtIndex(0,returnColumnAtIndex(1) - 1);
    break;
    case left:
      Snake::setRowAtIndex(0,returnRowAtIndex(1)); 
      Snake::setColumnAtIndex(0,returnColumnAtIndex(1) + 1);
    break;
  }
}
void Snake::setDirection(direction newDirection) {
  snakeDirection = newDirection;
}
direction Snake::returnDirection() {
  return snakeDirection;
}
void Snake::setLength(int newLength) {
  length = newLength;
}
int Snake::returnLength() {
  return length;
}
void Snake::setRowAtIndex(int index, int row) {
  body[index].row = row;
}
int Snake::returnRowAtIndex(int index) {
  return body[index].row;
}
void Snake::setColumnAtIndex(int index, int column) {
  body[index].column = column;
}
int Snake::returnColumnAtIndex(int index) {
  return body[index].column;
}