//MKHALLATI Hassan 000459680
#include "lib.h"

#define INIT_CAPACITY 5

void delete_table(structArray* Array)
{
	for (int i = 0; i<Array->len; i++) free(Array->table[i].pseudo);
	free(Array->table);
	free(Array);
}

structArray* init_memory()
{
	/*
    initialise de la structure, alloue un espace memoire de 5 joueurs et initialise la capacité du tableau
	Retourne  la structure en cas de succès, NULL en cas d'échec
	*/
	structArray* Table = NULL;
	if ((Table = (structArray*)malloc(sizeof(structArray))) == NULL)
		{ //erreur d'allocation de mémoire primaire (struct)
			return NULL;
		}
	else if ((Table->table = (player*) malloc(sizeof(player)*INIT_CAPACITY)) == NULL)
		{// erreur d'allocation secondaire (tableau)
			delete_table(Table);
			return NULL;
		}
	Table->len = 0;
	Table->capacity = INIT_CAPACITY;
	return Table;
}

int add_player(structArray* Array, char* string, int*indexPlayer)
{
	/*
	Rajoute le struct joueur au tableau. Réallocation de la mémoire si nécessaire. Les clés du tableau sont les id des joueurs (afin d'obtenir une recherche rapide des joueurs)
	Retourne  la structure en cas de succès, NULL en cas d'échec
	*/
	int index;
	for (index = 0; index < Array->len; index++){
		if (!strcmp(Array->table[index].pseudo, string)) break;
	}
	if (index == Array->capacity){
		//Dépassement mémoire
		Array->capacity *=2;
		player* tmp_table = (player*) realloc(Array->table, Array->capacity*sizeof(player));
		if (!tmp_table)
		{ //erreur de réallocation de mémoire
			printf("Echec de réallocation de mémoire \n");
			return -1;
		}
		Array->table = tmp_table;
		Array->len += 1;
		Array->table[index].pseudo = string;
		Array->table[index].score = 0;
		*indexPlayer =  index;
		printf("réalloction et ajout du joueur %s\n", Array->table[index].pseudo);
	} else if(index==Array->len){
		Array->len += 1;
		Array->table[index].pseudo = string;
		Array->table[index].score = 0;
		*indexPlayer = index;
		printf("Ajout du joueur %s\n", Array->table[index].pseudo);
	} else {
		*indexPlayer = index;
	}
	return 1;
}


void imprimer_liste(structArray* Array){
	/*
	renvoie le tableau sur la console
	*/
	printf("impression du tableau\n");
	for (int i = 0;i< Array->len;i++)  printf("pseudo :%s %d\n", Array->table[i].pseudo, Array->table[i].score);
}
