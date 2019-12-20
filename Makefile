#MKHALLATI Hassan 000459680
CC = gcc
CFLAGS = -Wall -Werror -Wpedantic -std=c11

anagram.o: anagram.c
	$(CC) $(CFLAGS) -c anagram.c
lib.o: lib.c
	$(CC) $(CFLAGS) -c lib.c
personal_strings.o: personal_strings.c
	$(CC) $(CFLAGS) -c personal_strings.c
projet_client.o: projet_client.c
	$(CC) $(CFLAGS) -c projet_client.c
projet_serveur.o: projet_serveur.c
	$(CC) $(CFLAGS) -c projet_serveur.c -lpthread
cli: projet_client.o personal_strings.o
	$(CC) $(CFLAGS) projet_client.o personal_strings.o -o maranga_cli
srv: projet_serveur.o anagram.o personal_strings.o lib.o
	$(CC) $(CFLAGS) lib.o personal_strings.o anagram.o projet_serveur.o -lpthread -o maranga_srv 
clean :
	rm *.o
