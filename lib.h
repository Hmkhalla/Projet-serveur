//MKHALLATI Hassan 000459680
#ifndef LIB_H
#define LIB_H
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
	char* pseudo;
	int score;
} player;

typedef struct
{
	int len;
	int capacity;
	player* table;
} structArray;

int add_player(structArray* table, char* pseudo, int *indexPlayer);
/*
	Rajoute le struct joueur au tableau et retourne le pointeur de la structure.
	Retourne le pointeur NULL en cas d'erreur et libère la mémoire précedement allouée.
*/
/*
void addPoint(structArray* table, int *indexPlayer);*/

structArray* init_memory();
/*
    Initialise de la structure tableau, alloue un espace memoire de 5 joueurs et retourne le pointeur de la structure.
	Retourne le pointeur NULL en cas d'erreur et libère la mémoire précedement allouée.
*/

void imprimer_liste(structArray* table);
/*
	Renvoie le tableau sur la console
*/

void delete_table(structArray* table);
/*
    Libère la mémoire accordée au tableau.
*/


#endif
