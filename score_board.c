#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "score_board.h"

// Open score file and build linked list.
// If the file doesn't exist, don't do anything

char *SCOREFILE = "BoggleScoreFile";
LLNode *LLHead;

void open_score_file(){
    FILE *score_file = fopen(SCOREFILE, "r");

    // initialize the LLHead to NULL since we do not know if exists players
    // to populate the list
    LLHead = NULL;
    if (score_file != NULL){
        LLNode *temp_head = malloc(sizeof(LLNode));
        int error = fread(temp_head, sizeof(LLNode), 1, score_file);
        if (error == 1){

            // Get rid of the junk that might be stored at temp_head->next
            temp_head->next = NULL;
            LLNode *next_node;

            // Build the linked list
            while (error == 1){
                next_node = temp_head;
                temp_head = malloc(sizeof(LLNode));
                error = fread(temp_head, sizeof(LLNode), 1, score_file);
                temp_head->next = next_node;
            }

            // Free the junk that was not read by fread
            free(temp_head);
            LLHead = next_node;
        } else {
            free(temp_head);
        }
        fclose(score_file);
    } else {
        fprintf(stderr, "Caution: Score file does not exist or could "
        "not be opened. This is normal if running the game for the first "
        "time.\n");
    }
}

void display_scores(){
    LLNode *temp_node = LLHead;
    fprintf(stdout, "Player Scores:\n");
    while(temp_node != NULL){
        fprintf(stdout, "User: %s\nMax Score: %d\nTotal Games: %d\nTotal Score: %d\n\n",
                temp_node->name, temp_node->max_score,
                temp_node->total_games, temp_node->total_score);
        temp_node = temp_node->next;
    }
}

LLNode *find_player(char *search_name){
    LLNode *temp_node = LLHead;
    while(temp_node != NULL){
        if(strcmp(temp_node->name, search_name) == 0){
            return temp_node;
        }
        temp_node = temp_node->next;
    }

    // Player not found
    return NULL;
}

LLNode *create_new_player(char *name){
    LLNode *new_player = malloc(sizeof(LLNode));
    strncpy(new_player->name, name, 19);
    new_player->max_score = 0;
    new_player->total_games = 0;
    new_player->total_score = 0;
    new_player->next = LLHead;
    LLHead = new_player;
    return new_player;
}

void write_score_file(){
    FILE *score_file = fopen(SCOREFILE, "wb");
    if (!score_file){
        fprintf(stderr, "Couldn't open file to write scores.\n");
        exit(1);
    }
    int error;
    LLNode *temp_node = LLHead;

    // Iterate through the linked list, writing to score_file at each iteration
    while(temp_node != NULL){
        error = fwrite(temp_node, sizeof(LLNode), 1, score_file);
        if (error != 1){
            fprintf(stderr, "Couldn't write a linked list node to file.\n");
            exit(1);
        }
        temp_node = temp_node->next;
    }
    fclose(score_file);
}

void clean_up_list(){
    LLNode *temp_head = LLHead;
    LLNode *last_node;

    // Iterate through the linked list, freeing the previously seen node
    // at each iteration
    while (temp_head != NULL){
        last_node = temp_head;
        temp_head = temp_head->next;
        free(last_node);
    }
}
