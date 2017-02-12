#include "server.h"

// The heads of the linked lists that will be maintained.
static struct player *top_player = NULL;
static struct client *top_client = NULL;

/* Handler for SIGALRM. Generates a new game board and informs
clients that a new game has started. */
void handler(int code) {

	// Build a new game board
	initialize_dice_members();
    Dice *dice_arr = roll_dice();
    set_game_board(dice_arr);

    // Broadcast the current game is over message to all clients
    inform_new_game();
}

/* Informs all clients that a new game has started,
and thus a new game board created. Sets each clients profiles ingame field
to 0 (not in game).*/
void inform_new_game(){
	struct client *cur_client = top_client;
	char out_buf[BUF_SIZE] = "A new game has begun.\r\n";
	while(cur_client != NULL){

		// Unsure if the code should have this functionality
		// if (cur_client->profile != NULL){
		// 	cur_client->profile->in_game = 0;
		// }
		if(write(cur_client->fd, out_buf, strlen(out_buf) * sizeof(char)) < 0){
			perror("write");
			exit(1);
		}
		cur_client = cur_client->next;
	}
}

/* Sends the game board to the client pointed at by cur_client. */
void new_game_command(struct client *cur_client){
	if (cur_client->profile->in_game == 1){
		char out_buf[BUF_SIZE] = "Cannot start a new game since a game is already in progress.\r\n";
		if(write(cur_client->fd, out_buf, strlen(out_buf) * sizeof(char)) < 0){
			perror("write");
			exit(1);
		}
		return;
	}
	cur_client->profile->in_game = 1;
	display_board(cur_client->fd);
}

/* Writes a list of all players on the server with their number of games played,
total score, and max score to the file descriptor clientfd.*/
void all_players_command(int clientfd) {
	struct player *cur_player = top_player;
	char out_buf[BUF_SIZE];
	while(cur_player != NULL){
		sprintf(out_buf, "%s: Games Played: %d, Total Score: %d, Max Score: %d\r\n",
				cur_player->name, cur_player->num_games, cur_player->total_score,
				cur_player->max_score);
		if(write(clientfd, out_buf, strlen(out_buf) * sizeof(char)) < 0){
			perror("write");
			exit(1);
		}
		cur_player = cur_player->next;
	}
}

/* Writes the top 3 players on the server to the file descriptor clientfd.*/
void top_3_command(int clientfd) {

	// max_heap will be maintained so that max_heap[0] >= max_heap[1] >= max_heap[2]
	// and these will be the top 3 clients that have been seen at any given iteration
	struct player *max_heap[3] = {NULL};
	struct player *cur_player = top_player;
	char out_buf[BUF_SIZE];
	int i = 0;

	max_heap[0] = cur_player;
	cur_player = cur_player->next;

	while(cur_player != NULL){
		if (cur_player->max_score >= max_heap[0]->max_score){
			max_heap[2] = max_heap[1];
			max_heap[1] = max_heap[0];
			max_heap[0] = cur_player;
		} else if ((max_heap[1] == NULL) ||
				   (cur_player->max_score >= max_heap[1]->max_score)){
			max_heap[2] = max_heap[1];
			max_heap[1] = cur_player;
		} else if ((max_heap[2] == NULL) ||
				   (cur_player->max_score >= max_heap[2]->max_score)){
			max_heap[2] = cur_player;
		}
		cur_player = cur_player->next;
	}

	while(i < 3){
		if(max_heap[i] != NULL){
			sprintf(out_buf, "%i. %s, Score: %d\r\n", i + 1,
				max_heap[i]->name, max_heap[i]->max_score);
			if(write(clientfd, out_buf, strlen(out_buf) * sizeof(char))
				< strlen(out_buf) * sizeof(char)){
				perror("write");
				exit(1);
			}
		}
		++i;
	}
}

/* Adds score to the client pointed at by cur_client if and only if the client
has previously executed the new_game command.*/
int add_score_command(struct client *cur_client, int score) {
	if ((cur_client->profile)->in_game == 1){
		(cur_client->profile)->in_game = 0;
		(cur_client->profile)->total_score += score;
		(cur_client->profile)->num_games += 1;
		if((cur_client->profile)->max_score < score){
			(cur_client->profile)->max_score = score;
		}
		return 1;
	}
	return 0;
}

/* Removes the client associated with clientfd from the server
and write "Goodbye." to the client.*/
void quit_command(int clientfd) {
	struct client *cur_client = top_client;
	struct client *next_client;
	char writebuf[BUF_SIZE];
	if(cur_client == NULL){
		fprintf(stderr, "An unexpected error occured. This should never happen.\n");
		exit(1);
	}
	sprintf(writebuf, "Goodbye.\r\n");
	if(write(clientfd, writebuf, strlen(writebuf)) < 0){
		perror("write");
		exit(1);
	}
	if(cur_client->fd == clientfd){
		top_client = cur_client->next;
		if (cur_client->profile != NULL){
			cur_client->profile->in_game = 0;
		}
		free(cur_client);
	} else {
		while(cur_client->next != NULL){
			if((cur_client->next)->fd == clientfd){
				next_client = cur_client->next;
				cur_client->next = (cur_client->next)->next;
				if (next_client->profile != NULL){
					next_client->profile->in_game = 0;
				}
				free(next_client);
				break;
			}
			cur_client = cur_client->next;
		}
	}
	if (close(clientfd) < 0){
		perror("close");
		exit(1);
	}
}

/* Writes an error message to the clientfd file descriptor informing a
client of an invalid command.*/
void inform_command_error(int clientfd) {
	char message[BUF_SIZE];
	sprintf(message, "Invalid command. Note that add_score can only be issued once after new_game.\r\n");
	if(write(clientfd, message, strlen(message)) < 0){
		perror("write");
		exit(1);
	}
}

/* Searches the first inbuf characters of buf for a network newline ("\r\n").
  Return the location of the '\r' if the network newline is found,
  or -1 otherwise. Code was written completely by us in Lab 11. */
int find_network_newline(const char *buf, int inbuf) {
  int n = 0;
  while (n < inbuf){
      if (buf[n] == '\r' && buf[n + 1] == '\n'){
          return n;
      }
      ++n;
  }
  return -1; // return the location of '\r' if found
}

/* Executes socket(), bind(), and listen() with error checking.*/
int setup_in_socket(struct sockaddr_in *sockaddress) {
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("perror");
		exit(1);
	}

	int on = 1;
    int status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                            (const char *) &on, sizeof(on));
    if(status == -1) {
        perror("setsockopt -- REUSEADDR");
    }

	if(bind(sockfd, (struct sockaddr *)sockaddress, sizeof(*sockaddress))){
		perror("bind");
		exit(1);
	}

	if(listen(sockfd, 10)){
		perror("listen");
		exit(1);
	}
	return sockfd;
}

/* A wrapper for the accept command that performs error checking.*/
int accept_new_client(struct sockaddr_in *clientaddress, int sockfd) {
	int clientfd;
	socklen_t client_addr_len = sizeof(*clientaddress);
	if((clientfd = accept(sockfd, (struct sockaddr *)clientaddress,
						  &client_addr_len)) < 0){
		perror("accept");
		exit(1);
	}
	return clientfd;
}

/* Searches the list of clients for a client with a file descriptor
equal to fd.*/
struct client *search_client_by_fd(int fd) {
	if (top_client != NULL){
		struct client *cur_client = top_client;
		while(cur_client != NULL){
			if(cur_client->fd == fd){
				return cur_client;
			}
			cur_client = cur_client->next;
		}
	}
	return NULL;
}

/* Sets the profile of the client associated with clientfd to point to a
player with the same name or, if such a player does not exist,
creates and adds a new player with the client name.*/
void link_client_player(int clientfd) {
	struct client *cur_client = search_client_by_fd(clientfd);
	char *username = cur_client->name;
	if (top_player != NULL){
		struct player *cur_player = top_player;
		while(cur_player != NULL){
			if (strcmp(cur_player->name, username) == 0){
				cur_client->profile = cur_player;
				return;
			}
			cur_player = cur_player->next;
		}
	}
	struct player *new_player = malloc(sizeof(struct player));
	if (new_player == NULL){
		perror("malloc");
		exit(1);
	}
	strncpy(new_player->name, username, MAXNAME);
	new_player->num_games = 0;
	new_player->in_game = 0;
	new_player->total_score = 0;
	new_player->max_score = 0;
	new_player->next = top_player;
	top_player = new_player;
	cur_client->profile = top_player;
	return;
}

/* Creates a client with fd as its fd and adds it to the client linked list.*/
void add_client(int fd) {
	struct client *new_client = calloc(1, sizeof(struct client));
	if (new_client == NULL){
		perror("calloc");
		exit(1);
	}
	new_client->fd = fd;
	new_client->next = top_client;
	top_client = new_client;
}

/* Assumes that the client associated with clientfd has a name in their inbuf.
Sets the clients name to the string in the inbuf, notifies the client of any
truncating that occured, and notifies the client of the login.*/
void process_client_username(int clientfd) {
	char writebuf[BUF_SIZE];
	struct client *cur_client = search_client_by_fd(clientfd);
	if (cur_client->buf_ptr - cur_client->inbuf + 1 > MAXNAME){
		strcpy(writebuf, "Your name was too long and was truncated.\r\n");
		if(write(clientfd, writebuf, strlen(writebuf)) < 0){
			perror("write");
			exit(1);
		}
		cur_client->inbuf[MAXNAME] = '\0';
	}
	strcpy(cur_client->name, cur_client->inbuf);
	sprintf(writebuf, "Client %s logged in.\r\n", cur_client->name);
	if(write(clientfd, writebuf, strlen(writebuf)) < 0){
		perror("write");
		exit(1);
	}
	cur_client->buf_ptr = cur_client->inbuf;
}

/* Sends a greeting message to the client associated with clientfd.*/
void greet_client(int clientfd) {
	char greetmsg[BUF_SIZE];
	sprintf(greetmsg, "Welcome to our server, please enter your username.\r\n");
	if(write(clientfd, greetmsg, strlen(greetmsg)) < 0){
		perror("write");
		exit(1);
	}
}

/* Read as much input as is available from the client associated with clientfd.
Returns the number of bytes read.*/
int read_client_input(int clientfd) {
	struct client *cur_client = search_client_by_fd(clientfd);
	int nbytes;
	if (cur_client->buf_ptr == NULL){
		cur_client->buf_ptr = cur_client->inbuf;
	}

	if((nbytes = read(cur_client->fd, cur_client->buf_ptr,
					  BUF_SIZE - (cur_client->buf_ptr - cur_client->inbuf))) < 0){
		perror("read");
		exit(1);
	}
	cur_client->buf_ptr += nbytes;
	return nbytes;
}

/* Returns 1 if the client associated with clientfd has a network newline in its inbuf,
0 otherwise.*/
int check_client_input(int clientfd){
	int newline;
	struct client *cur_client = search_client_by_fd(clientfd);
	if ((newline = find_network_newline(cur_client->inbuf,
									   cur_client->buf_ptr - cur_client->inbuf)) >= 0){
		cur_client->inbuf[newline] = '\0';
		return 1;
	}
	return 0;
}

/* Assumes that the client associated with clientfd has a valid command according
to check_client_input. Returns 1 if successful in launching command, 0 otherwise.*/
int interpret_command(int clientfd) {
	struct client *cur_client = search_client_by_fd(clientfd);
	if(strcmp(cur_client->inbuf, "all_players") == 0){
		all_players_command(cur_client->fd);
		cur_client->buf_ptr = cur_client->inbuf;
		return 1;
	} else if(strstr(cur_client->inbuf, "add_score") != NULL){
		// Extracting the score
		char *space_ptr = strstr(cur_client->inbuf, " ");
		if (space_ptr == NULL){
			inform_command_error(clientfd);
			cur_client->buf_ptr = cur_client->inbuf;
			return 0;
		}
		int add_score = strtol(space_ptr + 1, NULL, 10);
		if (add_score_command(cur_client, add_score) == 0){
			inform_command_error(cur_client->fd);
		}
		cur_client->buf_ptr = cur_client->inbuf;
		return 1;
	} else if(strcmp(cur_client->inbuf, "quit") == 0){
		quit_command(cur_client->fd);
		return 1;
	} else if (strcmp(cur_client->inbuf, "top_3") == 0){
		top_3_command(clientfd);
		cur_client->buf_ptr = cur_client->inbuf;
		return 1;
	} else if (strcmp(cur_client->inbuf, "new_game") == 0){
		new_game_command(cur_client);
		cur_client->buf_ptr = cur_client->inbuf;
		return 1;
	}
	inform_command_error(clientfd);
	cur_client->buf_ptr = cur_client->inbuf;
	return 0;
}

/* Returns the larger of the maximum client file descriptor and sockfd.*/
int max_fd(int sockfd){
	struct client *cur_client = top_client;
	int max_fd = sockfd;
	while(cur_client != NULL){
		if (cur_client->fd > max_fd){
			max_fd = cur_client->fd;
		}
		cur_client = cur_client->next;
	}
	return max_fd;
}

int main() {
	int sockfd, clientfd, readnum;
	struct sockaddr_in sockaddress;

	// New game alarm setup
	struct itimerval timer;
    struct sigaction sa;

    sa.sa_handler = handler;
	sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGALRM);
    sigaction(SIGALRM, &sa, NULL);

    timer.it_value.tv_sec = TIME;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = TIME;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);

	// Socket setup
	memset(&sockaddress, '\0', sizeof(sockaddress));
	sockaddress.sin_family = AF_INET;
	sockaddress.sin_addr.s_addr = htonl(INADDR_ANY);
	sockaddress.sin_port = htons(PORT);

	sockfd = setup_in_socket(&sockaddress);
	int maxfd;
	struct client *cur_client, *temp_client;

	/* Taken from muffinman.c from here*/
	fd_set fdlist;
	FD_ZERO(&fdlist);
	FD_SET(sockfd, &fdlist);
	/* to here */

	// Build new game board
	initialize_dice_members();
    Dice *dice_arr = roll_dice();
    set_game_board(dice_arr);

	while(1){
		FD_ZERO(&fdlist);
		FD_SET(sockfd, &fdlist);
		cur_client = top_client;
		while(cur_client != NULL){
			FD_SET(cur_client->fd, &fdlist);
			cur_client = cur_client->next;
		}
		maxfd = max_fd(sockfd);
		if (select(maxfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
			if(errno == EINTR){
				continue;
			} else {
				perror("select");
	        	exit(1);
			}
    	}
    	cur_client = top_client;
    	while(cur_client != NULL){
    		if (FD_ISSET(cur_client->fd, &fdlist)){
    			readnum = read_client_input(cur_client->fd);

    			// If the client has not yet been linked to a player
    			if (cur_client->profile == NULL &&
    				readnum > 0 && check_client_input(cur_client->fd) != 0){
    				process_client_username(cur_client->fd);
    				link_client_player(cur_client->fd);
    				cur_client = cur_client->next;

    			// If the client has issued a command
    			} else if (readnum > 0 && check_client_input(cur_client->fd) != 0){

    				// The client could issue the quit command,
    				// which would make cur_client no longer a valid pointer
    				temp_client = cur_client->next;
    				interpret_command(cur_client->fd);
    				cur_client = temp_client;

    			// If the client has disconnected
    			} else if (readnum == 0){

    				// The client will be removed and cur_client will
    				// no longer be a valid pointer
    				temp_client = cur_client->next;
    				quit_command(cur_client->fd);
    				cur_client = temp_client;
    			}
    		} else {
    			cur_client = cur_client->next;
    		}
    	}
    	if (FD_ISSET(sockfd, &fdlist)){
    		struct sockaddr_in clientaddress;
    		clientfd = accept_new_client(&clientaddress, sockfd);
    		greet_client(clientfd);
    		add_client(clientfd);
    	}
	}
	return 0;
}
