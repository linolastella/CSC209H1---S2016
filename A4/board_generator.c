#include "board_generator.h"

GameBoard gb;
int dice_numbers[16];
char dice_letters[16][6];

void initialize_dice_members(){
    // initialize numbers
    int a;
    for (a = 0; a < 16; a++) {
        dice_numbers[a] = a;
    }

    // initialize letters
    strncpy(dice_letters[0], "RIFOBX", 6);
    strncpy(dice_letters[1], "IFEHEY", 6);
    strncpy(dice_letters[2], "DENOWS", 6);
    strncpy(dice_letters[3], "UTOKND", 6);
    strncpy(dice_letters[4], "HMSRAO", 6);
    strncpy(dice_letters[5], "LUPETS", 6);
    strncpy(dice_letters[6], "ACITOA", 6);
    strncpy(dice_letters[7], "YLGKUE", 6);
    strncpy(dice_letters[8], "QBMJOA", 6);
    strncpy(dice_letters[9], "EHISPN", 6);
    strncpy(dice_letters[10], "VETIGN", 6);
    strncpy(dice_letters[11], "BALIYT", 6);
    strncpy(dice_letters[12], "EZAVND", 6);
    strncpy(dice_letters[13], "RALESC", 6);
    strncpy(dice_letters[14], "UWILRG", 6);
    strncpy(dice_letters[15], "PACEMD", 6);
}

Dice *roll_dice() {
    // randomize dice numbers
    int i, j, temp;
    for (i = 15; i >= 0; i--) {
        srand(time(NULL));
        j = rand() % (i + 1);
        temp = dice_numbers[i];
        dice_numbers[i] = dice_numbers[j];
        dice_numbers[j] = temp;
    }

    // randomize dice letters
    Dice *dice_array = (Dice *) malloc(sizeof(Dice) * 16);
    Dice die;
    int k;
    for (k = 0; k < 16; k++) {
        srand(time(NULL));
        int r = rand() % 6;
        die.number = dice_numbers[k]; // dice_numbers has been randomized before
        die.letter = dice_letters[die.number][r];
        dice_array[k] = die;
    }
    return dice_array;
}

void set_game_board(Dice *dice_arr) {
    int row, column, count;
    count = 0;
    for (row = 0; row < 4; row++) {
        for (column = 0; column < 4; column++) {
            gb.board[row][column] = dice_arr[count];
            count++;
        }
    }
}

void display_board(int sendfd) {
	int a, b;
    char send_str[5];
	for (a = 0; a < BOARD_SIZE; a++) {
		for (b = 0; b < BOARD_SIZE; b++) {
            sprintf(send_str, "%c ", gb.board[a][b].letter);
			if(write(sendfd, send_str, strlen(send_str) * sizeof(char)) < 0){
                perror("write");
                exit(1);
            }
		}
        sprintf(send_str, "\r\n");
        if(write(sendfd, send_str, strlen(send_str) * sizeof(char)) < 0){
            perror("write");
            exit(1);
        }
	}
}
