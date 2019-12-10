CC = gcc
CPPFLAGS=
CFLAGS = -Wall -Werror -Wpedantic -std=c99 -c
LDFLAGS = 

maranga_test: maranga_test.o anagram.o personal_strings.o
	$(CC) $(LDFLAGS) -o $@ $^

maranga_test.o: maranga_test.c
	$(CC) $(CFLAGS) maranga_test.c

anagram.o: anagram.c
	$(CC) $(CFLAGS) anagram.c

personal_strings.o: personal_strings.c
	$(CC) $(CFLAGS) personal_strings.c
