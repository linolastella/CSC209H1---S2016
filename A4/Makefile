PORT=57793
CFLAGS= -DPORT=\$(PORT) -std=c99 -g -Wall -Werror

game: server.c board_generator.c
	gcc ${CFLAGS} -o game_server server.c board_generator.c
