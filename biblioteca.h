#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <math.h>
#include <stdio.h>
#define BUFLEN 256

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)


typedef struct tcp_message {
	char id[10];
	char topic[50];
	unsigned char type; /*1 pt subscribe, 0- unsubscribe*/
	unsigned char SF;
}message_tcp;


typedef struct udp_message {
    char topic[50];
    unsigned char data_type;
    unsigned char payload[1500];
}message_udp;

typedef struct sv_to_tcp_message {
	char topic[50];
	unsigned char data_type;
	unsigned char payload[1500];
	uint32_t value0; /*pentru cazul 0 = INT*/
	float value12; /*pt cazurile 1 si 2, short real si float*/
	char ip[100];
	int port;
}message_to_tcp;

typedef struct LS {
	message_to_tcp msg;
	struct LS *urms;
}TLStore, *TListaS;

typedef struct LC {
	int sockfd;
	unsigned char sf;
	char id_client[10];
	TListaS store; //Lista de mesaje to store
	struct LC *urmc;
} TLClient, *TListaC;

typedef struct LT {
	char topic[50];
	TListaC info;
	struct LT *urm;
}TLT, *TLista, **ALista;

TListaS AlocStore(message_to_tcp msg);
TListaC AlocClient(char id[10], unsigned char sf, int sockfd);
TLista AlocTopic(char topic[50]);
void ins_store(TListaS *aL, message_to_tcp msg);
void ins_topic(ALista aL, char topic[50]);
void ins_client(TListaC *aL, char id[10], unsigned char sf, int sockfd);
void ins_client_in_topic(ALista aL, char topic[50], char id[10], unsigned char sf, int sockfd);
void delete_msg(TListaS *aL);
void delete_client(TListaC *aL, char id[10]);
void delete_client_from_topic(TLista L, char topic[50], char id[10]);
void afis(TLista aL);
int exist_client(TListaC L, char id[10]);
int exist_topic(TLista L, char topic[50]);
int exist_client_in_topic(TLista L, char topic[50], char id[10]);
char *get_client_id(TListaC L, int sockfd);
TLista get_topic(TLista L, char topic[50]);
void destroy(ALista aL);
void destroy_clients(TListaC *aL);
void destroy_store(TListaS *store);
void update_socket(ALista aL, char id[10], int sockfd);