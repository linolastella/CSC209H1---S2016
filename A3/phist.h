#ifndef PHIST_H
#define PHIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

#define MAXLINE 1100
#define MAXWORD 45
#define MAXFILES 400
#define USAGE_MSG "Usage : phist -d <input file dir> -n <number of processes>  -o <output file name>\n"

void file_count (int *word_counts, char *file_name);
int dir_search(char **file_names, char *directory);
void child_write(char **file_names, int *pipes, int id, int N, int num_files);
void parent_read(int *word_counts, int *pipes, int N);
void write_output(char *out_file, int *word_counts);

#endif
