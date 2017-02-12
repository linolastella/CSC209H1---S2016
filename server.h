#ifndef SERVER_H
#define SERVER_H

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include "board_generator.h"

#define MAXNAME 50
#define BUF_SIZE 1000
#define TIME 120

#ifndef PORT
  #define PORT 57792
#endif

typedef struct player {
	char name[MAXNAME + 1];
	int num_games;
	int in_game; /* 1 if yes, 0 otherwise */
	int total_score;
	int max_score;
	struct player *next;
} Player;

typedef struct client {
    int fd;
    char name[MAXNAME + 1];
	char inbuf[BUF_SIZE];

	// Points to the first char after what has been read into inbuf
	char *buf_ptr;
    struct player *profile;
    struct client *next;
} Client;

void inform_new_game();
void new_game_command(struct client *cur_client);
int interpret_command(int clientfd);
void all_players_command(int clientfd);
void top_3_command(int clientfd);
int add_score_command(struct client *cur_client, int score);
void quit_command(int clientfd);
void inform_command_error(int clientfd);

int find_network_newline(const char *buf, int inbuf);

int setup_in_socket(struct sockaddr_in *sockaddress);
int max_fd();

int accept_new_client(struct sockaddr_in *clientaddress, int sockfd);
int read_client_input(int clientfd);
void add_client(int fd);
void process_client_username(int clientfd);
void greet_client(int clientfd);
void link_client_player(int clientfd);
struct client *search_client_by_fd(int fd);

#endif
