CC = gcc
CFLAGS = -Wall -Werror -Wpedantic -std=c99 -c 

anagram.o: anagram.c
	$(CC) $(CFLAGS) anagram.c
lib.o: lib.c
	$(CC) $(CFLAGS) lib.c
personal_strings.o: personal_strings.c
	$(CC) $(CFLAGS) personal_strings.c
projet_client.o: projet_client.c
	$(CC) $(CFLAGS) projet_client.c
projet_serveur.o: projet_serveur.c
	$(CC) $(CFLAGS) projet_serveur.c -lpthread
cli: projet_client.o personal_strings.o
	$(CC) $(CFLAGS) projet_client.o personal_strings.o -o maranga_cli
srv: projet_serveur.o anagram.o personal_strings.o lib.o
	$(CC) $(CFLAGS) lib.o personal_strings.o anagram.o projet_serveur.o -o maranga_srv -lpthread

clean :
	rm *.O
	rm maranga_srv
	rm maranga_cli
