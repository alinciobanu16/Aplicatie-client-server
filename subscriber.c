#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "biblioteca.h"


void usage(char *file) {
	fprintf(stderr, "Usage: %s id_client server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[]) {
	int sockfd, n, ret, yes = 1;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];
	message_tcp msg1;
	message_to_tcp msg3;

	if (argc < 4) {
		usage(argv[0]);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(int));
	DIE(ret < 0, "setsockopt");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	fd_set read_fds;
	fd_set tmp_fds;
	int fdmax;
	FD_SET(sockfd, &read_fds);
	FD_SET(0, &read_fds);
	fdmax = sockfd;

	memset(&msg1, 0, sizeof(message_tcp));
	strcpy(msg1.id, argv[1]);
	/*trimit un mesaj cu id-ul sa stie server-ul ce client s-a conectat*/
	n = send(sockfd, &msg1, sizeof(message_tcp), 0);
	DIE (n < 0, "send");

	while (1) {
		tmp_fds = read_fds; 
		select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		if (FD_ISSET(0, &tmp_fds)) {
	  		// se citeste de la tastatura
	  		memset(&msg1, 0, sizeof(message_tcp));
	  		strcpy(msg1.id, argv[1]);
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin);

			if (strncmp(buffer, "exit", 4) == 0) {
				break;
			}

			char *p = NULL;
			p = strtok(buffer, " ");
			if (strcmp(p, "subscribe") == 0) {
				msg1.type = 1;
				p = strtok(NULL, " \n");
				if (p == NULL) {
					printf("Comanda gresita: prea putine argumente!\n");
					continue;
				}
				if (strlen(p) > 50) {
					printf("Topic gresit: mai mult de 50 de caractere\n");
					continue;
				}
				strcpy(msg1.topic, p);
				p = strtok(NULL, " \n");
				if (p == NULL) {
					printf("Comanda gresita: prea putine argumente!\n");
					continue;
				}
				if (strcmp(p, "1") != 0 && strcmp(p, "0") != 0) {
					printf("SF gresit\n");
					p = NULL;
					continue;
				}
				msg1.SF = p[0] - '0';
				p = strtok(NULL, "\n");
				if (p != NULL) {
					printf("Comanda gresita: prea multe argumente!\n");
					p = NULL;
					continue;
				}

			} else if (strcmp(p, "unsubscribe") == 0) {
				msg1.type = 0;
				p = strtok(NULL, " \n");
				if (p == NULL) {
					printf("Comanda gresita: prea putine argumente!\n");
					continue;
				}
				strcpy(msg1.topic, p);
				p = strtok(NULL, " \n");
				if (p != NULL) {
					printf("Comanda gresita: prea multe argumete!\n");
					p = NULL;
					continue;
				}
			} else {
				printf("Comanda invalida!\n");
				p = NULL;
				continue;
			}

			n = send(sockfd, &msg1, sizeof(message_tcp), 0);
			DIE(n < 0, "send");
			if (msg1.type == 1) {
				printf("subscribed %s\n", msg1.topic);
			} else if (msg1.type == 0) {
				printf("unsubscribed %s\n", msg1.topic);
			}
		}

		if (FD_ISSET(sockfd, &tmp_fds)) {
			//primesc de la server
			memset(&msg3, 0, sizeof(message_udp));
			n = recv(sockfd, &msg3, sizeof(message_to_tcp), 0);
        	DIE(n < 0, "recv");
        	if (n == 0) { /*server-ul a inchis conexiunea*/
        		break;
        	}
        	if (msg3.data_type == 0) {
        		printf("%s:%d - %s - INT - %d\n", msg3.ip, msg3.port, msg3.topic, msg3.value0);
        	}
        	if (msg3.data_type == 1) {
        		printf("%s:%d - %s - SHORT_REAL - %f\n", msg3.ip, msg3.port, msg3.topic, msg3.value12);
        	}
        	if (msg3.data_type == 2) {
        		printf("%s:%d - %s - FLOAT - %f\n", msg3.ip, msg3.port, msg3.topic, msg3.value12);
        	}
        	if (msg3.data_type == 3) {
        		printf("%s:%d - %s - STRING - %s\n", msg3.ip, msg3.port, msg3.topic, msg3.payload);
        	}
        }
	}

	close(sockfd);

	return 0;
}