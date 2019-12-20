//MKHALLATI Hassan 000459680
#include "anagram.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  init_anagram
 *  Description:  Initialise une structure anagram_t avec que des champs NULL.
 *   Parameters:  anagram_t * anagram: un pointeur sur la structure à initialiser
 *       Return:  
 * =====================================================================================
 */
/* --------- init_anagram --------- {{{ */
    void
init_anagram (anagram_t *anagram)
{

    anagram->liste = empty_pstring ();
    anagram->mot_courant = empty_pstring ();
    anagram->longueur_mot = 0;
    anagram->nombre_mot = 0;
    anagram->deb_mot_courant = NULL;
}		/* -----  end of function init_anagram  ----- */
/* }}} */


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  free_anagram
 *  Description:  Libère la mémoire allouée pour une structure de type anagram_t
 *   Parameters:  anagram_t anagram: la structure à libérer
 *       Return:  
 * =====================================================================================
 */
/* --------- free_anagram --------- {{{ */
    void
free_anagram (anagram_t *anagram)
{
    free_pstring (&(anagram->liste));
    free_pstring (&(anagram->mot_courant));

    anagram->deb_mot_courant = NULL;
    anagram->longueur_mot = 0;
    anagram->nombre_mot = 0;
}		/* -----  end of function free_anagram  ----- */
/* }}} */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  new_anagram
 *  Description:  Génére une nouvelle manche d'anagramme en utilisant le fichier dict
 *                  - anagram_t *a : un pointeur sur la structure amenée à contenir les 
 *                      informations d'une nouvelle manche
 *                  - char *dict   : le fichier contenant le dictionnaire d'anagrammes
 *       Return:  Retourne le nombre de réponses possibles en cas de succès, retourne -1
 *                sinon.
 * =====================================================================================
 */
/* --------- new_anagram --------- {{{ */
int new_anagram (anagram_t * a, dict_t *dico) 
{
    char *parser;                               /* un parser utilsé pour les manipulations de chaînes */
    int line;                                   /* la ligne de l'anagramme sélectionné */
    int ret_val;                                /* stocke les retours de fonction */
    FILE *dict_file;

    dict_file = fopen (dico->chemin.str, "r");
    if (!dict_file) 
    {
        return -1;
    }

    // On tire la ligne à considérer au hasard
    line = rand () % dico->nombre_lignes;

    // On parse le fichier jusqu'à la ligne qui nous intéresse
    while (line --)
    {
        secure_read_line (&(a->liste), dict_file);
    }

    // Lit la ligne courante
    ret_val = secure_read_line (&(a->liste), dict_file);
    if (ret_val == -1)
    {
        /*
         * fclose est susceptible de modifier la valeur de errno, on la stocke donc avant son appel
         * puis on réassigne errno à sa valeur initiale, afin de permettre à l'utilisateur de la
         * bibliothèque de pouvoir utiliser perror et d'être sûr que le message d'erreur est
         * approprié.
         */
        int tmp_errno = errno;
        fclose(dict_file);
        errno = tmp_errno;
        return -1;
    }

    // Fermeture du fichier car à partir de maintenant, il n'est plus utilisé
    fclose (dict_file);

    // Compte le nombre de mot et leur taille
    parser = a->liste.str;

    // On calcule la taille du premier segment de la chaîne qui ne contient aucun des caractères \n
    // \0 et ' '. Il s'agit donc de la taille d'un mot
    a->longueur_mot = strcspn(a->liste.str, " \n\0"); 

    // Compte le nombre d'anagramme dans la chaîne
    parser += a->longueur_mot;
    a->nombre_mot = 1; 

    while (*parser) 
    {
        parser += a->longueur_mot + 1;          /* tous les anagrammes ont la même taille, */
        a->nombre_mot ++;
    }

    // Sélection aléatoire du mot
    a->deb_mot_courant = a->liste.str + (rand() % a->nombre_mot)*(a->longueur_mot + 1);

    // Copie le mot dans le buffer
    ret_val = ncopy_string (&(a->mot_courant), a->deb_mot_courant, a->longueur_mot);
    if (ret_val == -1)
    {
        return -1;
    }

    return a->nombre_mot - 1;
}
/* }}} */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  is_solution
 *  Description:  Teste si une chaîne de caractère est une solution à la manche en cours
 *                  - const anagram_t *a : pointeur sur la manche en cour
 *                  - const char *chaine : chaîne de caractère à tester
 *       Return:  1 si la réponse est valide, zéro sinon
 * =====================================================================================
 */
/* --------- is_solution --------- {{{ */
int is_solution     (const anagram_t *a, const char *chaine) 
{
    // Dans un premier temps on teste la taille
    if (strlen(chaine) != a->longueur_mot)
        return 0;
    // La chaîne ne peut pas contenir d'espace
    // pour éviter tout problème avec la recherche de sous chaîne, on retourne 0 si la chaîne
    // fournie en contient au moins 1.
    if (strchr(chaine, ' '))
        return 0;

    char *souschaine = strstr(a->liste.str, chaine);
    if (souschaine && souschaine != a->deb_mot_courant)
        return 1;

    return 0;
}
/* }}} */



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  init_dico
 *  Description:  Initialise une structure dictionnaire, 
 *                  - dict_t *dico       :  la structure à initialiser
 *                  - const char *chemin :  le chemin du fichier dictionnaire
 *       Return:   Le nombre de lignes du fichier en cas de succès, -1 sinon.
 * =====================================================================================
 */
/* --------- init_dico --------- {{{ */
int init_dico(dict_t *dico, const char *chemin)
{
    pstring_t buf = {.str = NULL, .len = 0, .size = 0};
    int ret_val;

    dico->chemin = empty_pstring ();
    dico->nombre_lignes = 0;

    // La fonction copy_string initialise la mémoire au besoin
    ret_val = copy_string (&(dico->chemin), chemin);
    if (ret_val == -1)
    {
        return -1;
    }

    FILE *fichier_dict = fopen (dico->chemin.str, "r");
    if (!fichier_dict) 
    {
        return -1;
    }

    while (secure_read_line (&buf, fichier_dict))
    {
        dico->nombre_lignes++;
    }

    fclose(fichier_dict);

    free_pstring (&buf);

    /*-----------------------------------------------------------------------------
     *  Initialisation du random
     *-----------------------------------------------------------------------------*/
    srand(time(NULL));

    return dico->nombre_lignes;
}
/* }}} */



/*
 * ===  FUNCTION  ======================================================================
 *         Name:  free_dico
 *  Description:  Libère la mémoire allouée à un dictionnaire 
 *   Parameters:  dict_t *dico: la structure dictionnaire
 *       Return:  
 * =====================================================================================
 */
/* --------- free_dico --------- {{{ */
    void
free_dico (dict_t *dico)
{
    free_pstring (&(dico->chemin));
    dico->nombre_lignes = 0;
}		/* -----  end of function free_dico  ----- */
/* }}} */
