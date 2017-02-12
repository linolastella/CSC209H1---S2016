#ifndef BOARD_GENERATOR_H
#define BOARD_GENERATOR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

typedef struct dice {
  int number;
  char letter;
} Dice;

typedef struct game_board {
  Dice board[4][4];
} GameBoard;

extern GameBoard gb;

void initialize_dice_members();
Dice *roll_dice();
void set_game_board();
void start_game_session();

#endif
