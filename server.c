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
#include <netinet/tcp.h>
#include <math.h>
#include <stdio.h>

void usage(char *file) {
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

void create_msg(message_to_tcp *msg, char ip[100], int port, char topic[50], int data_type, unsigned char payload[1500]) {
	strcpy(msg->ip, ip);
	msg->port = port;
	strcpy(msg->topic, topic);
	msg->data_type = data_type;
	strcpy((char *)msg->payload, (char*)payload);
}

void send_stored_msg(TLista topics, char id_client[10], int sockfd) {
	TLista t;
	TListaC p;
	TListaS s;
	message_to_tcp msg;
	memset(&msg, 0, sizeof(message_to_tcp));
	if (topics != NULL) {
		for (t = topics; t; t = t->urm) {
			if (t->info) {
				for (p = t->info; p; p = p->urmc) {
					if (strcmp(p->id_client, id_client) == 0) {
						s = p->store;
						while (s) {
							memset(&msg, 0, sizeof(message_to_tcp));
							msg = s->msg;
							int n = send(sockfd, &msg, sizeof(message_to_tcp), 0);
							DIE(n < 0, "send");
							delete_msg(&s);
						}
						p->store = NULL;
					}
				}
			}
		}
	}
}

int main(int argc, char* argv[]) {
	struct sockaddr_in sockaddr_udp, from_udp;
	struct sockaddr_in sockaddr_tcp, cli_addr;
 
	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax = 0, ret, i = 0, newsockfd, yes = 1;
	char ip[100];
	message_tcp msg1;
	message_udp msg2;
	message_to_tcp msg3;
	TLista topics = NULL, t; /*lista de topicuri in care tin toti clientii abonati*/
	TListaC clients = NULL, p; /*lista de clienti in care tin toti clientii conectati la server*/

	socklen_t socket_len = sizeof(struct sockaddr_in);
	socklen_t clilen, addrlen = sizeof(from_udp);

	if (argc < 2) {
		usage(argv[0]);
	}

	int sockudp = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(sockudp < 0, "sockudp");

	int socktcp = socket(AF_INET, SOCK_STREAM, 0);
	DIE(socktcp < 0, "socktcp");

	ret = setsockopt(socktcp, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(int));
	if (ret < 0) {
		perror("setsockopt(TCP_NODELAY) failed");
	}

	ret = setsockopt(socktcp, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (ret < 0) {
		perror("setsockopt(SO_REUSEADDR) failed");
	}

	memset(&sockaddr_udp, 0, sizeof(sockaddr_udp));
	memset(&sockaddr_tcp, 0, sizeof(sockaddr_tcp));

	sockaddr_udp.sin_family = AF_INET;
	sockaddr_udp.sin_port = htons(atoi(argv[1]));
	sockaddr_udp.sin_addr.s_addr = INADDR_ANY;

	sockaddr_tcp.sin_family = AF_INET;
	sockaddr_tcp.sin_port = htons(atoi(argv[1]));
	sockaddr_tcp.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockudp, (struct sockaddr *)&sockaddr_udp, socket_len);
	DIE(ret < 0, "bind");
	
	ret = bind(socktcp, (struct sockaddr *)&sockaddr_tcp, socket_len);
    DIE(ret < 0, "bind");

    ret = listen(socktcp, 7);
    DIE(ret < 0, "listen");

    FD_SET(socktcp, &read_fds);
    FD_SET(sockudp, &read_fds);
    FD_SET(0, &read_fds);
	fdmax = socktcp;

    while (1) {
    	tmp_fds = read_fds;
    	ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
    	DIE(ret < 0, "select");

    	if (FD_ISSET(0, &tmp_fds)) {
    		char buff[BUFLEN];
    		fgets(buff, BUFLEN - 1, stdin);
    		if (strncmp(buff, "exit", 4) == 0) {
    			printf("Server-ul s-a inchis\n");
    			break;
    		} else {
    			printf("Comanda invalida!\n");
    			continue;
    		}
    	}

    	for (i = 0; i <= fdmax; i++) {
    		if (FD_ISSET(i, &tmp_fds)) {
				if (i == socktcp) {
					/*se conecteaza un nou client tcp*/
					clilen = sizeof(cli_addr);
					newsockfd = accept(socktcp, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd < 0, "accept");
					ret = setsockopt(newsockfd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(int));
					DIE(ret < 0, "setsockopt");

					memset(&msg1, 0, sizeof(message_tcp)); 
					ret = recv(newsockfd, &msg1, sizeof(message_tcp), 0); /*primesc un mesaj cu id-ul clientului nou*/
					DIE(ret < 0, "recv");
					if (exist_client(clients, msg1.id)) {
						printf("Client %s already connected!\n", msg1.id);
						close(newsockfd);
						continue;
					}

					update_socket(&topics, msg1.id, newsockfd); /*modific socketul in lista de topicuri*/
					inet_ntop(AF_INET, &(cli_addr.sin_addr.s_addr), ip, 100);
					printf("New client %s connected from %s:%d.\n", msg1.id, ip, ntohs(cli_addr.sin_port));
					ins_client(&clients, msg1.id, msg1.SF, newsockfd);

					send_stored_msg(topics, msg1.id, newsockfd); /*trimit mesajele stocate*/
					FD_SET(newsockfd, &read_fds);
					if (newsockfd > fdmax) { 
						fdmax = newsockfd;
					}
				}

				else if (i == sockudp) {
					/*primesc mesaj de pe socketul udp de la un client udp*/
					/*iterez prin lista de topicuri si trimit mesaj clientilor abonati*/
					memset(&msg2, 0, sizeof(message_udp));
					ret = recvfrom(sockudp, &msg2, sizeof(message_udp), 0, (struct sockaddr *)&from_udp, &addrlen);
		        	DIE(ret < 0, "recvfrom");
		        	if (ret == 0) {
		        		break;
		        	}

		        	if (msg2.data_type == 0) {
		        		uint32_t value = 0;
		        		value = (msg2.payload[1] << 24) + (msg2.payload[2] << 16) + (msg2.payload[3] << 8) + (msg2.payload[4]);

		        		if (msg2.payload[0] == 1) {
		        			value = -value;
		        		}

		        		memset(&msg3, 0, sizeof(message_to_tcp));
		        		memset(ip, 0, sizeof(ip));
		        		inet_ntop(AF_INET, &(from_udp.sin_addr.s_addr), ip, 100);
		        		create_msg(&msg3, ip, ntohs(from_udp.sin_port), msg2.topic, msg2.data_type, msg2.payload);
		        		msg3.value0 = value;
		        		if (topics != NULL) {
			        		t = get_topic(topics, msg2.topic); /*caut in lista de topicuri topicul din mesajul curent*/
			        		if (t != NULL && t->info != NULL) {
			        			for (p = t->info; p; p = p->urmc) { /*ma uit la clientii abonati la topic*/
			        				if (exist_client(clients, p->id_client)) { /*daca am clienti conectati pe server*/
			        					ret = send(p->sockfd, &msg3, sizeof(message_to_tcp), 0);
			        					DIE(ret < 0, "send");
			        				} else if (p->sf == 1) { /*daca clientul nu este conectat la server
			        					dar are optiunea de store activa ii stochez mesajele*/
			        					ins_store(&p->store, msg3);
			        				}
			        			}
			        		}
			        	}
		        	}

		        	if (msg2.data_type == 1) {
		        		uint16_t value = 0;
		        		float val = 0;
		        		value = (msg2.payload[0] << 8) + (msg2.payload[1]);
		        		val = (float)value / 100;
		        		memset(&msg3, 0, sizeof(message_to_tcp));
		        		memset(ip, 0, sizeof(ip));
		        		inet_ntop(AF_INET, &(from_udp.sin_addr.s_addr), ip, 100);
		        		create_msg(&msg3, ip, ntohs(from_udp.sin_port), msg2.topic, msg2.data_type, msg2.payload);
		        		msg3.value12 = val;
		        		if (topics != NULL) {
			        		t = get_topic(topics, msg2.topic);
			        		if (t != NULL && t->info != NULL) {
			        			for (p = t->info; p; p = p->urmc) {
			        				if (exist_client(clients, p->id_client)) {
			        					ret = send(p->sockfd, &msg3, sizeof(message_to_tcp), 0);
			        					DIE(ret < 0, "send");
			        				} else if (p->sf == 1) {
			        					//ii stochez mesajele
			        					ins_store(&p->store, msg3);
			        				}
			        			}
			        		}
			        	}
		        	}

		        	if (msg2.data_type == 2) {
		        		uint32_t value = 0;
		        		float val = 0;
		        		value = (msg2.payload[1] << 24) + (msg2.payload[2] << 16) + (msg2.payload[3] << 8) + (msg2.payload[4]);
		        		val = value * pow(10, -msg2.payload[5]);
		        		if (msg2.payload[0] == 1) {
		        			val = -val;
		        		}

		        		memset(&msg3, 0, sizeof(message_to_tcp));
		        		memset(ip, 0, sizeof(ip));
		        		inet_ntop(AF_INET, &(from_udp.sin_addr.s_addr), ip, 100);
		        		create_msg(&msg3, ip, ntohs(from_udp.sin_port), msg2.topic, msg2.data_type, msg2.payload);
		        		msg3.value12 = val;
		        		if (topics != NULL) {
			        		t = get_topic(topics, msg2.topic);
			        		if (t != NULL && t->info != NULL) {
			        			for (p = t->info; p; p = p->urmc) {
			        				if (exist_client(clients, p->id_client)) {
			        					ret = send(p->sockfd, &msg3, sizeof(message_to_tcp), 0);
			        					DIE(ret < 0, "send");
			        				} else if (p->sf == 1) {
			        					//ii stochez mesajele
			        					ins_store(&p->store, msg3);
			        				}
			        			}
			        		}
			        	}
		        	}

		        	if (msg2.data_type == 3) {
		        		memset(&msg3, 0, sizeof(message_to_tcp));
		        		memset(ip, 0, sizeof(ip));
		        		inet_ntop(AF_INET, &(from_udp.sin_addr.s_addr), ip, 100);
		        		create_msg(&msg3, ip, ntohs(from_udp.sin_port), msg2.topic, msg2.data_type, msg2.payload);
		        		if (topics != NULL) {
			        		t = get_topic(topics, msg2.topic);
			        		if (t != NULL && t->info != NULL) {
			        			for (p = t->info; p; p = p->urmc) {
			        				if (exist_client(clients, p->id_client)) {
			        					send(p->sockfd, &msg3, sizeof(message_to_tcp), 0);
			        					DIE(ret < 0, "send");
				        			} else if (p->sf == 1) {
			        					//ii stochez mesajele
			        					ins_store(&p->store, msg3);
			        				}
			        			}
			        		}
			        	}
		        	}

				} else {
					//s-au primit date pe unul din socketii client tcp
					//trebuie sa ii adaug intr-o lista
					memset(&msg1, 0, sizeof(message_tcp));
					ret = recv(i, &msg1, sizeof(message_tcp), 0);
					DIE(ret < 0, "recv");

					if (ret == 0) {
						//conexiunea s-a inchis
						printf("Client %s disconnected.\n", get_client_id(clients, i));
						delete_client(&clients, get_client_id(clients, i)); /*il sterg din lista de clienti activi pe server*/
						close(i);
						FD_CLR(i, &read_fds);

					} else {
						if (msg1.type == 1) { //vrea sa dea subscribe la un topic
							ins_topic(&topics, msg1.topic); //adaug in lista topicul
							ins_client_in_topic(&topics, msg1.topic, msg1.id, msg1.SF, i); //adaug clientul
						} else {
							//vrea sa dea unsubscribe, deci il elimin de la topicul respectiv
							delete_client_from_topic(topics, msg1.topic, msg1.id);
						}
					}
				}
			}
    	}
    }


    destroy(&topics);
    destroy_clients(&clients);

    close(socktcp);
    close(sockudp);

	return 0;
}
