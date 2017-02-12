#ifndef BOARD_GENERATOR_H
#define BOARD_GENERATOR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define BOARD_SIZE 4
#define BOARD_AREA 16

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
void set_game_board(Dice *dice_arr);
void display_board(int sendfd);

#endif
