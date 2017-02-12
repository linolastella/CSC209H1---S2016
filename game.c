#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "dictionary.h"
#include "game_hash_table.h"
#include "game.h"
#include "board_generator.h"
#include "score_board.h"
#include "word_checker.h"

int BOARD_SIZE = 4;
int BOARD_AREA = 16;

int compute_score(char *word) {
	int word_len = strlen(word);
	if (word_len == 3 || word_len == 4) {
		return 1;
	} else if (word_len == 5) {
		return 2;
	} else if (word_len == 6) {
		return 3;
	} else if (word_len == 7) {
		return 5;
	} else {
		return 11;
	}
}

char *prompt_username() {
	char username[20];
	fprintf(stdout, "Please enter username: ");
	fscanf(stdin, "%s", username);
	int len_username = strlen(username);
	char *return_name = malloc(sizeof(char) * len_username + 1);
	int h;
	for (h = 0; h < len_username; h++) {        // Convert to lower case
		return_name[h] = tolower(username[h]);
	}
	return_name[len_username] = '\0';
	return return_name;
}

void display_board() {
	int a, b;
	for (a = 0; a < BOARD_SIZE; a++) {
		for (b = 0; b < BOARD_SIZE; b++) {
			fprintf(stdout, "%c ", gb.board[a][b].letter);
		}
		fprintf(stdout, "\n");
	}
}

void new_game() {
	system("clear");
	open_score_file();
	display_scores();
	setup_dictionary();
	start_game_session();
	int i;
	char input_word[15] = "1";
	int is_valid_word;
	int temp_score = 0;
	int curr_session_score = 0;
	char *username;
	int input_word_len;
	LLNode *player;
	while (strcmp(input_word, "q") != 0 && strcmp(input_word, "n") != 0) {
		display_board();
		if (strcmp(input_word, "1") != 0) {
			input_word_len = strlen(input_word);

			// Convert input word to upper case
			for (i = 0; i < input_word_len; ++i){
				input_word[i] = toupper(input_word[i]);
			}

			is_valid_word = word_checker(gb, input_word);
			if (strlen(input_word) < 3) {
				fprintf(stdout, "Word must be of at least 3 characters!\n");
				is_valid_word = 0;
			}
			if (is_valid_word) {
				temp_score = compute_score(input_word);
				fprintf(stdout, "You entered: %s. Points earned: %d\n",
								input_word, temp_score);
				curr_session_score += temp_score;
			} else {
				fprintf(stdout, "\"%s\" is not an English word or has already "
								"been used\n", input_word);
			}
		}

		fprintf(stdout, "Enter a valid word ('q' to quit the program, 'n' "
						"for a new game session): ");
		fscanf(stdin, "%s", input_word);
		system("clear");
	}
	if (strcmp(input_word, "q") == 0) {  // Exit the program
		write_score_file();
		display_scores();
		fprintf(stdout, "Game exiting, bye!\n");
		teardown_dictionary();
		clean_up_list();
		return;
	} else {  // Start a new session
		fprintf(stdout, "Your score for this session is: %d\n",
						 curr_session_score);
		username = prompt_username();
		LLNode *search_user = find_player(username);

		// If this player has played before
		if (search_user != NULL){
			player = search_user;
		} else {
			player = create_new_player(username);
		}
		free(username);
		if (curr_session_score > player->max_score) {
			player->max_score = curr_session_score;
		}

		player->total_score += curr_session_score;
		player->total_games += 1;
		write_score_file();
		display_scores();
		teardown_dictionary();
		clean_up_list();
		new_game();
	}
}

int main (int argc, char **argv) {
	if (argc == 1) {
		new_game();
	} else {
		setup_dictionary();
		FILE *test_file = fopen(argv[1], "r");
		FILE *output_file = fopen("result.txt", "w");
		if (test_file != NULL && output_file != NULL) {
			int test_score = 0;
			char test_board[BOARD_AREA];
			char test_inputs[MAX_LINE];
			if (fgets(test_board, BOARD_AREA + 1, test_file) != NULL) {
				// Build game board
				int i, j;
				int count = 0;
				for (i = 0; i < BOARD_SIZE; i++) {
					for (j = 0; j < BOARD_SIZE; j++) {
						gb.board[i][j].letter = test_board[count];
						count++;
					}
				}
				// Read inputs
				fgets(test_inputs, MAX_LINE, test_file);
				fgets(test_inputs, MAX_LINE, test_file);
				test_inputs[strcspn(test_inputs, "\r\n")] = '\0'; // Strip end-of-line
				char *input_ptr;
				int input_ptr_len;
				int flag = 0;    // flag is 1 if there is at least one invalid input
				for (input_ptr = strtok(test_inputs, ","); input_ptr != NULL; input_ptr = strtok(NULL, ",")) {
					input_ptr_len = strlen(input_ptr);
					for (i = 0; i < input_ptr_len; ++i){
						input_ptr[i] = toupper(input_ptr[i]);
					}

					int is_valid = word_checker(gb, input_ptr);
					if (is_valid) {
						test_score += compute_score(input_ptr);
					} else if (flag) {
						fprintf(output_file, ",%s", input_ptr);
					} else {
						fprintf(output_file, "%s", input_ptr);
						flag = 1;
					}
				}
				fprintf(output_file, "\n");
				fprintf(output_file, "%d\n", test_score);
			}
		} else {
			fprintf(stderr, "Error opening file.\n");
			exit(1);
		}
		teardown_dictionary();
		int error = fclose(output_file) + fclose(test_file);
		if (error) {
			fprintf(stderr, "fclose failed!\n");
			exit(1);
		}
	}
	return 0;
}
