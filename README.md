# Snake Game 
Realized with Arduino Nano, 8x8 SPI LED matrix, 16x2 LCD and a joystick.

---
## About

Code for the game is written in C++ for ArduinoIDE.
- 'GameBase' includes the 'Snake' class and 'Dot' struct (one pixel on the matrix).
- 'SnakeGame' includes the game logic.
- 'SnakeSPI' is the run code for Arduino boards.

After the game is completed, based on the player's score, the score can be
written to EEPROM of Arduino.

Player three character name is input from the Serial monitor.

Three character name and the score are written to EEPROM.

These scores are sorted and only the best three scores are written to EEPROM
in the end.

---
## Photos

Circuit on a breadboard
<div align="center"> <img src="/images and pdfs/circuit/circuit_on_a_breadboard.jpg"> </div>

In game state
<div align="center"> <img src="/images and pdfs/gameplay/4. in_game_state_1.jpg"> </div>

Game over and final score
<div align="center"> <img src="/images and pdfs/gameplay/6. game_over_state.jpg"> </div>

Username input
<div align="center"> <img src="/images and pdfs/gameplay/7. name_input.jpg"> </div>

Scoreboard
<div align="center"> <img src="/images and pdfs/gameplay/12. scoreboard_three_scores.jpg"> </div>
