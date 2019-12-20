//MKHALLATI Hassan 000459680
#ifndef ANAGRAM_H
#define ANAGRAM_H

#include "personal_strings.h"
/*
 * =====================================================================================
 *    Structure:  anagram_t
 *  Description:  Structure contenant les informations nécessaires à un anagramme
 *                  - char liste[]          : chaîne de caractère contenant un mot et 
 *                                              tous ses anagrammes
 *                  - char mot_courant[]    : chaîne de caractère contenant une copie de 
 *                                              l'anagramme en cours
 *                  - char *deb_mot_courant : pointeur sur le début du mot proposé aux 
 *                                              clients
 *                  - int longueur_mot      : la longueur du mot et de ses anagrammes
 *                  - int nombre_mot        : le nombre de mot que contient la ligne
 * =====================================================================================
 */
typedef struct anagram {
    pstring_t liste, mot_courant;
    char *deb_mot_courant;
    int  longueur_mot, nombre_mot;
} anagram_t;


/*
 * =====================================================================================
 *    Structure:  dict_t
 *  Description:  Structure contenant les informations relatives à un dictionnaire
 *                  - char chemin[]     : chaîne de caractères contenant le chemin vers 
 *                                          le dictionnaire
 *                  - int nombre_lignes : nombre d'entrées du dictionnaire
 * =====================================================================================
 */
typedef struct dict {
    pstring_t chemin;
    int  nombre_lignes;
} dict_t;


void init_anagram (anagram_t *);
void free_anagram (anagram_t *);

int new_anagram (anagram_t *, dict_t *);
int is_solution     (const anagram_t *, const char *);

int init_dico      (dict_t *, const char *);
void free_dico           (dict_t *); 
#endif
