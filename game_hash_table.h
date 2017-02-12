#include "dictionary.h"
#ifndef GAME_HASH_TABLE_H
#define GAME_HASH_TABLE_H

extern DNode **WORD_DICTIONARY;
extern DNode **SEEN_DICTIONARY;
extern int MAX_LINE;

void setup_dictionary();
void teardown_dictionary();

#endif
