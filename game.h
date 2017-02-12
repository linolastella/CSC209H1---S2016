#ifndef GAME_H
#define GAME_H

extern int BOARD_SIZE;
extern int BOARD_AREA;

void new_game();
int compute_score(char *);
char *prompt_username();
void display_board();
#endif
