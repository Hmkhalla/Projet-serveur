//MKHALLATI Hassan 000459680
#include "lib.h"
#define INIT_CAPACITY 2

void delete_table(structArray* Array)
{
	/*
    libère la mémoire du tableau de joueur et ensuite la structure tableau
	*/
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

structArray* add_player(structArray* Array, char* string, int*indexPlayer)
{
	/*
	Rajoute le struct joueur au tableau. Réallocation de la mémoire si nécessaire. Les clés du tableau sont les id des joueurs (afin d'obtenir une recherche rapide des joueurs)
	Retourne  la structure en cas de succès, NULL en cas d'échec
	*/
	int index;
	for (index = 0; index < Array->len; index++){
		printf("entry :%s tableau :%s\n", string, Array->table[index].pseudo);
		if (!strcmp(Array->table[index].pseudo, string)){
			printf("find !\n"); break;
		}
	}
	printf("index %d\n", index);
	if (index == Array->capacity){
		//Dépassement mémoire
		Array->capacity *=2;
		if ((Array->table = (player*) realloc(Array->table, Array->capacity*sizeof(player))) == NULL)
		{ //erreur de réallocation de mémoire
			delete_table(Array);
			return NULL;
		}
		Array->len += 1;
		Array->table[index].pseudo = string;
		Array->table[index].score = 0;
		*indexPlayer =  index;
		//printf("réalloction d'un joueur, index%d, len %d, capa %d\n", index, Array->len, Array->capacity);
		printf("réalloction du joueur %s, index %d, score %d, len %d, capa %d\n", Array->table[index].pseudo, index,Array->table[index].score, Array->len, Array->capacity);
	} else if(index==Array->len){
		Array->len += 1;
		Array->table[index].pseudo = string;
		Array->table[index].score = 0;
		*indexPlayer = index;
		//printf("ajout d'un joueur, index%d, len %d, capa %d\n", index, Array->len, Array->capacity);
		printf("ajout du joueur %s, index %d, score %d, len %d, capa %d\n", Array->table[index].pseudo, index,Array->table[index].score, Array->len, Array->capacity);
	} else {
		*indexPlayer = index;
		printf("chargement du joueur %s, index %d, score %d, len %d, capa %d\n", Array->table[index].pseudo, index,Array->table[index].score, Array->len, Array->capacity);
	}
	return Array;
}


void imprimer_liste(structArray* Array){
	/*
	renvoie le tableau sur la console
	*/
	printf("impression du tableau\n");
	for (int i = 0;i< Array->len;i++)  printf("pseudo :%s %d\n", Array->table[i].pseudo, Array->table[i].score);
}
