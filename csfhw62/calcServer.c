#include <stdio.h>      /* for snprintf */
#include "csapp.h"
#include "calc.h"
#include <pthread.h>

/* buffer size for reading lines of input from user */
#define LINEBUF_SIZE 1024

void chat_with_client(struct Calc *calc, int infd, int outfd);
void *worker(void *arg);
//int done2 = 1;

struct Client {
	struct Calc *calc;
	int clientfd;
	//client socket file descriptor
};

void *worker(void *arg) {
	struct Client *client = arg;
	
	pthread_detach(pthread_self());
	chat_with_client(client->calc, client->clientfd, client->clientfd);
	close(client->clientfd);
	free(client);

	return NULL;
}

int main(int argc, char **argv) {
	// handle too many arguments and invalid arguments
	if (argc != 2) {
		printf("Invalid number of arguments\n");
		return 1;
	}
	int tcp_port = atoi(argv[1]);
	if (tcp_port < 1024) {
		printf("Invalid port\n");
		return 1;
	}

	struct Calc *calc = calc_create();
	
	//open server socket
	int serverfd = Open_listenfd(argv[1]);
	if (serverfd < 0) { //<0
		printf("Error opening port to listen");
		//calc_destroy(calc);
		return 1;
	}
	
	//int connfd = 0;
	//int shutdown = 0;

	//while shutdown command is not received
	while (1) { //shutdown != 2
		//int clientfd = Accept(serverfd, NULL, NULL);
		int clientfd = Accept(serverfd, NULL, NULL);
		if (clientfd < 0) {
			printf("Error accepting client connection");
			return 1;
		}
		struct Client *client = malloc(sizeof(struct Client));
		client->calc = calc;
		client->clientfd = clientfd;
		
		pthread_t thr_id;
		if (pthread_create(&thr_id, NULL, worker, client) != 0) {
			printf("pthread_create failed");
			return 1;
		}

		//Close(connfd);
	}

	//shutdown = chat_with_client(calc, connfd, connfd);
	Close(serverfd);
	calc_destroy(calc);
	return 0;
}

void chat_with_client(struct Calc *calc, int infd, int outfd) {
	rio_t in;
	char linebuf[LINEBUF_SIZE];

	/* wrap standard input (which is file descriptor 0) */
	rio_readinitb(&in, infd);

	/*
	 * Read lines of input, evaluate them as calculator expressions,
	 * and (if evaluation was successful) print the result of each
	 * expression.  Quit when "quit" command is received.
	 * Stop looking for connection when "shutdown" command is received
	 */
	int done = 0;
	while (!done) {
		ssize_t n = rio_readlineb(&in, linebuf, LINEBUF_SIZE);

		if (n <= 0) {
			/* error or end of input */
			done = 1;
		} else if (strcmp(linebuf, "quit\n") == 0 || strcmp(linebuf, "quit\r\n") == 0) {
			/* quit command */
			done = 1;
		} else if (strcmp(linebuf, "shutdown\n") == 0 || strcmp(linebuf, "shutdown\r\n") == 0) {
			/* quit command and end server connection */
			done = 1;
			//done2 = 0;
		} else {
			/* process input line */
			int result;
			if (calc_eval(calc, linebuf, &result) == 0) {
				/* expression couldn't be evaluated */
				rio_writen(outfd, "Error\n", 6);
			} else {
				/* output result */
				int len = snprintf(linebuf, LINEBUF_SIZE, "%d\n", result);
				if (len < LINEBUF_SIZE) {
					rio_writen(outfd, linebuf, len);
				}
			}
		}
	}
	//return 1;
}
