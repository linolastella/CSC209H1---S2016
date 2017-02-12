#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "dictionary.h"
#include "game_hash_table.h"

char *DICT_NAME = "EnglishWords.txt";
int MAX_LINE = 100;
DNode **WORD_DICTIONARY;
DNode **SEEN_DICTIONARY;

void setup_dictionary(){
    WORD_DICTIONARY = calloc(1, sizeof(DNode *) * BIG_HASH_SIZE);
    char line [MAX_LINE];
    int i;

    FILE *input_FP = fopen(DICT_NAME, "r");
    if(!input_FP){
        fprintf(stderr,
                "Could not open file \"%s\" for reading dictionary words\n",
                DICT_NAME);
        exit(1);
    }

    while(fgets(line, MAX_LINE, input_FP) != NULL){
        int word_end = strcspn(line, "\r\n");
		line[word_end] = '\0';
        for (i = 0; i < word_end; i++) {
    		line[i] = toupper(line[i]);  // Convert to upper case
    	}
		insert (WORD_DICTIONARY, BIG_HASH_SIZE, line);
	}
	fclose (input_FP);

    SEEN_DICTIONARY = calloc(1, sizeof(DNode *) * SMALL_HASH_SIZE);
}

void teardown_dictionary(){
    free_dictionary(WORD_DICTIONARY, BIG_HASH_SIZE);
    free_dictionary(SEEN_DICTIONARY, SMALL_HASH_SIZE);
    free(WORD_DICTIONARY);
    free(SEEN_DICTIONARY);
}
