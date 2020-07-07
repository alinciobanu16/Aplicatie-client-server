# Protocoale de comunicatii:
# Laborator 8: Multiplexare
# Makefile

CFLAGS = -Wall -g

# Portul pe care asculta serverul (de completat)
PORT = 

# Adresa IP a serverului (de completat)
IP_SERVER = 

all: server subscriber

# Compileaza server.c
server: server.c lista.c biblioteca.h -lm

# Compileaza client.c
subscriber: subscriber.c lista.c biblioteca.h

.PHONY: clean run_server run_client

# Ruleaza serverul
run_server:
	./server ${PORT}

# Ruleaza clientul
run_subscriber:
	./subscriber ${IP_SERVER} ${PORT}

clean:
	rm -f server subscriber
