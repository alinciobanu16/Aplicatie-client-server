#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "biblioteca.h"

TListaS AlocStore(message_to_tcp msg) {
	TListaS aux = (TListaS)calloc(1, sizeof(TLStore));
	if (!aux) return NULL;
	aux->msg = msg;
	aux->urms = NULL;
	return aux;
}

TListaC AlocClient(char id[10], unsigned char sf, int sockfd) {
	TListaC aux = (TListaC)calloc(1, sizeof(TLClient));
	if (!aux) return NULL;
	strcpy(aux->id_client, id);
	aux->sf = sf;
	aux->sockfd = sockfd;
	aux->store = NULL;
	aux->urmc = NULL;
	return aux;
}

TLista AlocTopic(char topic[50]) {
	TLista aux = (TLista)calloc(1, sizeof(TLT));
	if (!aux) return NULL;
	strcpy(aux->topic, topic);
	aux->info = NULL;
	aux->urm = NULL;
	return aux;
}

int exist_topic(TLista L, char topic[50]) {
	if (L == NULL) return 0;
	for (; L; L = L->urm) {
		if (strcmp(L->topic, topic) == 0) {
			return 1;
		}
	}
	return 0;
}

/*functie de inserare a unui topic, mereu la final*/
void ins_topic(ALista aL, char topic[50]) {
	TLista aux, u = NULL;
	if (*aL != NULL) {
		u = *aL;
		if (exist_topic(u, topic)) return;
		while (u->urm != NULL) {
			if (strcmp(u->topic, topic) == 0) {
				return;
			}
			u = u->urm;
		}
	}
	aux = AlocTopic(topic);
	if (!aux) return;
	if (u == NULL) {
		*aL = aux;
	} else {
		u->urm = aux;
	}
}

int exist_client(TListaC L, char id[10]) {
	if (L == NULL) return 0;
	for (; L; L = L->urmc) {
		if (strcmp(L->id_client, id) == 0) {
			return 1;
		}
	}
	return 0;
}

int exist_client_in_topic(TLista L, char topic[50], char id[10]) {
	if (L == NULL) return 0;
	TListaC p;
	for (; L; L = L->urm) {
		if (strcmp(L->topic, topic) == 0) {
			for (p = L->info; p; p = p->urmc) {
				if (strcmp(p->id_client, id) == 0) {
					return 1;
				}
			}
		}
	}
	return 0;
}

void ins_store(TListaS *aL, message_to_tcp msg) {
	TListaS aux, u = NULL;
	if (*aL != NULL) {
		u = *aL;
		while (u->urms != NULL) {
			u = u->urms;
		}
	}
	aux = AlocStore(msg);
	if (!aux) return;
	if (u == NULL) {
		*aL = aux;
	} else {
		u->urms = aux;
	}
}

void ins_client(TListaC *aL, char id[10], unsigned char sf, int sockfd) {
	TListaC aux, u = NULL;
	if (*aL != NULL) {
		u = *aL;
		if (exist_client(u, id)) return;
		while (u->urmc != NULL) {
			if (strcmp(u->id_client, id) == 0) {
				return;
			}
			u = u->urmc;
		}
	}
	aux = AlocClient(id, sf, sockfd);
	if (!aux) return;
	if (u == NULL) {
		*aL = aux;
	} else {
		u->urmc = aux;
	}
}

void ins_client_in_topic(ALista aL, char topic[50], char id[10], unsigned char sf, int sockfd) {
	TLista aux;
	aux = *aL;
	for (; aux; aux = aux->urm) {
		if (strcmp(aux->topic, topic) == 0) {
			break;
		}
	}

	ins_client(&aux->info, id, sf, sockfd);
}

void delete_msg(TListaS *aL) {
	if (*aL == NULL) return;
	TListaS aux;
	aux = *aL;
	*aL = aux->urms;
	free(aux);
}

void delete_client(TListaC *aL, char id[10]) {
	if (*aL == NULL) return;
	TListaC ant, p;
	for (p = *aL, ant = NULL; p != NULL; ant = p, p = p->urmc) {
		if (strcmp(p->id_client, id) == 0) {
			break;
		}
	}
	if (p == NULL) return;
	if (ant == NULL) {
		*aL = p->urmc;
		free(p);
	} else {
		ant->urmc = p->urmc;
		free(p);
	}
}

void delete_client_from_topic(TLista L, char topic[50], char id[10]) {
	if (L == NULL) return;
	TLista p = NULL;
	if (exist_client_in_topic(L, topic, id)) {
		for (p = L; p; p = p->urm) {
			if (strcmp(p->topic, topic) == 0) {
				break;
			}
		}
		
		delete_client(&p->info, id);
	}
}

void afis(TLista aL) {
	TLista L;
	TListaC p;
	TListaS s;
	for (L = aL; L; L = L->urm) {
		printf("topic: %s cu clientii \n[", L->topic);
		for (p = L->info; p; p = p->urmc) {
			printf("%s cu mesajele stocate:[", p->id_client);
			for (s = p->store; s; s = s->urms) {
				printf("%s, ", s->msg.topic);
			}
			printf("]\n");
		}
		printf("]\n");
	}
}

char *get_client_id(TListaC L, int sockfd) {
	for (; L; L = L->urmc) {
		if (L->sockfd == sockfd) {
			return L->id_client;
		}
	}
	return NULL;
}

TLista get_topic(TLista L, char topic[50]) {
	if (L == NULL) return NULL;
	for (; L; L = L->urm) {
		if (strcmp(L->topic, topic) == 0) {
			return L;
		}
	}
	return NULL;
}

void destroy(ALista aL) {
	TLista aux;
	while (*aL) {
		aux = *aL;
		*aL = aux->urm;
		destroy_clients(&aux->info);
		free(aux);
	}
}

void destroy_clients(TListaC *aL) {
	if (*aL == NULL) return;
	TListaC aux;
	while (*aL) {
		aux = *aL;
		*aL = aux->urmc;
		destroy_store(&aux->store);
		free(aux);
	}
}

void destroy_store(TListaS* store) {
	if (*store == NULL) return;
	TListaS aux;
	while (*store) {
		aux = *store;
		*store = aux->urms;
		free(aux);
	}
}

void update_socket(ALista aL, char id[10], int sockfd) {
	if (*aL == NULL) return;
	TLista aux = *aL;
	TListaC p;
	for (; aux; aux = aux->urm) {
		if (aux->info) {
			for (p = aux->info; p; p = p->urmc) {
				if (strcmp(p->id_client, id) == 0) {
					p->sockfd = sockfd;
				}
			}
		}
	}
}