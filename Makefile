CC = gcc
CPPFLAGS=
CFLAGS = -Wall -Werror -Wpedantic -std=c99
LDFLAGS = -lpthread

ifeq ($(DEBUG), 1)
	CPPFLAGS += -g
endif

all: parrot_client parrot_server 

#####################################
#  Client
#####################################
parrot_client: parrot_client.o
	$(CC) $(LDFLAGS) -o $@ $^

parrot_client.o: parrot_client.c



#####################################
#  Server
#####################################
parrot_server: parrot_server.o
	$(CC) $(LDFLAGS) -o $@ $^

parrot_server.o: parrot_server.c

