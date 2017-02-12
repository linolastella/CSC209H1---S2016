#include "phist.h"

/* Searches file and modifies word_counts to contain, at each index i,
the number of words in file with length i. word_counts[0] will default to 0.*/
void file_count(int *word_counts, char *file_name) {
    FILE *open_file;
    char line[MAXLINE + 1];
    char *word;
    int word_len;
    char delimiter[] = " ,.-!?\"|':;+";

    if ((open_file = fopen(file_name, "r")) == NULL) {
        perror("fopen");
        exit(1);
    }

    while (fgets(line, MAXLINE + 1, open_file) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        word = strtok (line, delimiter);
        while (word != NULL) {
            word_len = strlen(word);
            if (word_len <= MAXWORD) {
                word_counts[word_len] += 1;
            } else {
                fprintf(stderr, "Found word of invalid length (> %d), not counted\n",
                        MAXWORD);
            }
            word = strtok (NULL, delimiter);
        }
    }

    if (fclose(open_file) != 0){
        perror("fclose");
        exit(1);
    }
}

/* Populates file_names with all files names found in the path given by
 directory returns the number of files found. */
int dir_search(char **file_names, char *directory) {
    DIR *d;
    int num_files = 0;
    struct dirent *dir;

    d = opendir (directory);
    if (d) {
        while ((dir = readdir(d)) != NULL && num_files < MAXFILES) {
            if (dir->d_type == DT_REG) {
                file_names[num_files] = malloc(strlen(dir->d_name) * sizeof(char)
                                               + strlen(directory) + 4);
                strncpy(file_names[num_files], "./", 2);
                strncat(file_names[num_files], directory, strlen(directory));
                strncat(file_names[num_files], "/", 1);
                strncat(file_names[num_files], dir->d_name, strlen(dir->d_name));
                ++num_files;
            }
        }
        closedir (d);
    }
    return num_files;
}

/* Computes a range of files, based on id, of which to count word lengths
 from. Write the total number of word lengths into the write pipe in pipes,
 belonging to the child with id, in increasing order*/
void child_write(char **file_names, int *pipes, int id, int N, int num_files) {
    int word_counts[MAXWORD + 1] = {0};
    int i, n, error;
	if (id == (N - 1)) {
		for (i = id * (num_files / N); i < num_files; ++i) {
			file_count(word_counts, file_names[i]);
    	}
	} else {
		for (i = id * (num_files / N); i < (id + 1) * (num_files / N); ++i) {
			file_count(word_counts, file_names[i]);
		}
	}
    for (n = 1; n <= MAXWORD; ++n){
        if ((error = write(pipes[(id * 2) + 1],
        word_counts + n, sizeof(int))) < 0) {
            perror("write");
            exit(1);
        }
    }
}

/* Reads from pipes to collect all child data and builds word_counts based on
data*/
void parent_read(int *word_counts, int *pipes, int N) {
    int single_count = 0;
    int i, n, error;
    for (n = 1; n <= MAXWORD; ++n) {
        for (i = 0; i < N; ++i){
            error = read(pipes[2 * i], &single_count, sizeof(int));
            if (error < 0) {
                perror("read");
                exit(1);
            }
            word_counts[n] += single_count;
        }
    }
}

/* Write the output to a file */
void write_output(char *out_file, int *word_counts) {
    int i;
    FILE *out_file_ptr = NULL;
    if (out_file == NULL){
        out_file_ptr = stdout;
    } else {
        out_file_ptr = fopen(out_file, "w");
        if (out_file_ptr == NULL){
            perror("fopen");
            exit(1);
        }
    }
    for (i = 1; i <= MAXWORD; ++i) {
        fprintf(out_file_ptr, "%d, %d\n", i, word_counts[i]);
    }
    if (out_file_ptr != stdout) {
        fclose(out_file_ptr);
        if (out_file_ptr == NULL) {
            perror("fopen");
            exit(1);
        }
    }
}

int main(int argc, char **argv) {
    struct timeval starttime, endtime;
    double timediff;

    if ((gettimeofday(&starttime, NULL)) == -1) {
        perror("gettimeofday");
        exit(1);
    }

    int N = 1;
    char *in_dir = NULL;
    char *out_file = NULL;
    int num_files, n, i, error, close_check, id, ch;

    if (argc < 2) {
        fprintf(stderr, USAGE_MSG);
        exit(1);
    }
    while ((ch = getopt(argc, argv, "n:o:d:")) != -1) {
        switch(ch) {
            case 'o':
                out_file = optarg;
                break;
            case 'n':
                if (atoi(optarg) > 0) {
                    N = atoi(optarg);
                } else {
                    fprintf(stderr, USAGE_MSG);
                    exit(1);
                }
                break;
            case 'd':
                in_dir = optarg;
                break;
            case '?':
                fprintf(stderr, USAGE_MSG);
                exit(1);
            default:
                exit(1);
        }
    }
    if (in_dir == NULL) {
        fprintf(stderr, USAGE_MSG);
        exit(1);
    }

    char **file_names = malloc(sizeof(char *) * MAXFILES);
    if (file_names == NULL){
    	perror("malloc");
    	exit(1);
    }

    num_files = dir_search(file_names, in_dir);
    int word_counts[MAXWORD + 1] = {0};

    if (N < 2) {
        for (i = 0; i < num_files; i++) {
            file_count(word_counts, file_names[i]);
        }
        write_output(out_file, word_counts);
        while (num_files > 0) {
            	num_files--;
            	free(file_names[num_files]);
        }
        free(file_names);
        if ((gettimeofday(&endtime, NULL)) == -1) {
            perror("gettimeofday");
            exit(1);
        }
        timediff = (endtime.tv_sec - starttime.tv_sec) +
                   (endtime.tv_usec - starttime.tv_usec) / 1000000.0;
        fprintf(stderr, "%.4f\n", timediff);
        return 0;
    } else if (N > num_files) {
        N = num_files;
    }

    // Set up pipes
    int pipes[2 * N];
    for(n = 0; n < N; ++n) {
        error = pipe(pipes + (2 * n));
        if (error != 0) {
            perror("pipe");
            exit(1);
        }
    }

    error = 1;
    id = -1;
    for (n = 0; n < N; ++n) {
    	error = fork();
        if (error < 0) {
            perror("fork");
        	exit(1);
    	} else if (error == 0){
        	// Each child has a unique ID and parent has -1
        	id = n;
        	close_check = close(pipes[2 * n]);
        	if (close_check != 0) {
        		perror("close");
        		exit(1);
        	}
        	break;
    	} else {
    		close_check = close(pipes[2 * n + 1]);
    		if (close_check != 0) {
        		perror("close");
        		exit(1);
        	}
    	}
    }

    // If a child process
    if (id >= 0){
    	child_write(file_names, pipes, id, N, num_files);
        while (num_files > 0) {
            	num_files--;
            	free(file_names[num_files]);
        }
        free(file_names);
        exit(0);
    // If parent process
    } else {
        parent_read(word_counts, pipes, N);
	    write_output(out_file, word_counts);

	    for (i = 0; i <= id; ++i) {
	    	if (wait(&error) == -1) {
                perror("wait");
                exit(1);
            }
            if (WIFEXITED(error) == 0){
                fprintf(stderr, "Error: a child process terminated prematurely.\n");
            } else if(WEXITSTATUS(error) == 1){
                fprintf(stderr, "Error: a child process exited with an error.\n");
            }
	    }
        if ((gettimeofday(&endtime, NULL)) == -1) {
            perror("gettimeofday");
            exit(1);
        }
        timediff = (endtime.tv_sec - starttime.tv_sec) +
                   (endtime.tv_usec - starttime.tv_usec) / 1000000.0;
        fprintf(stderr, "%.4f\n", timediff);
	}
	while (num_files > 0) {
        	num_files--;
        	free(file_names[num_files]);
    }
    free(file_names);
	return 0;
}
