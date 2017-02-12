#ifndef SCORE_BOARD_H
#define SCORE_BOARD_H

typedef struct llnode {
    char name[20];
    int max_score;
    int total_games;
    int total_score;
    struct llnode *next;
} LLNode;

extern char *SCOREFILE;
extern LLNode *LLHead;

void open_score_file();
void display_scores();
LLNode *find_player(char *);
LLNode *create_new_player(char *);
void write_score_file();
void clean_up_list();

#endif
