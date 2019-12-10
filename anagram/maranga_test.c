/*
 * =====================================================================================
 *
 *       Filename:  maranga_test.c
 *
 *    Description:  Un programme de test pour illustrer l'utilisation de la bibliothèque
 *                      d'anagrammes
 *
 *        Version:  1.0
 *        Created:  26/11/2019 16:06:19
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Guillerme Duvillié (mfreeze), guillerme@duvillie.eu
 *   Organization:  
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "personal_strings.h"
#include "anagram.h"

#define EXIT_CMD  "\\quit"

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  main function
 * =====================================================================================
 */
    int
main (int argc, char **argv)
{
    int ret_val;
    int exit_loop = 0;
    dict_t dico;
    anagram_t anagram;

    /*
     * La fontion empty_string permet d'initialiser une structure pstring_t.
     * Cette structure contient un pointeur sur caractère (l'espace mémoire dans lequel est stocké
     * la chaîne) ainsi que un champ contenant la longueur de la chaîne ainsi que la taille de
     * l'espace mémoire alloué.
     *
     * XXX Vous n'êtes pas obligé d'utiliser cette structure et pouvez parfaitement utiliser un
     * pointeur sur char *. Cette strucutre est simplement utilisée dans les anagrammes, mais vous
     * pouvez normalement tout faire sans.
     */
    pstring_t user_entry = empty_pstring ();

    if (argc != 2)
    {
        fprintf (stderr, "Le programme prend le chemin vers le dictionnaire en paramètre.\n");
        return EXIT_FAILURE;
    }

    // Dans un premier on initialise le dictionnaire
    ret_val = init_dico (&dico, argv[1]);
    if (ret_val == -1)
    {
        // Les fonctions des bibliothèques fournies font normalement en sorte que errno soit mis à
        // la bonne valeur en cas d'erreur.
        perror ("Erreur initialisation dictionnaire: ");
        return EXIT_FAILURE;
    }

    // L'initialisation de l'anagramme
    init_anagram (&anagram);

    // Bouvle principale
    while (!exit_loop)
    {
        // On génère un nouvel anagram
        ret_val = new_anagram (&anagram, &dico);
        if (ret_val == -1)
        {
            perror ("Création d'un nouvel anagramme: ");
            free_pstring (&user_entry);
            free_anagram (&anagram);
            free_dico (&dico);

            return EXIT_FAILURE;
        }

        // Boucle secondaire (on itère sur l'anagramme courant tant que l'utilisateur ne l'a pas
        // trouvé)
        do
        {
            fprintf (stdout, "Trouve l'anagramme de < %s >: ", anagram.mot_courant.str);

            // Lecture de l'entrée standard, pour cette fonction il faut utiliser une structure
            // pstring afin de profiter d'un agrandissement au besoin de la mémoire allouée pour la
            // chaîne de caractères
            ret_val = secure_read_line (&user_entry, stdin);
            if (ret_val == -1)
            {
                perror ("Lecture entrée client: ");
                free_pstring (&user_entry);
                free_anagram (&anagram);
                free_dico (&dico);

                return EXIT_FAILURE;
            }


            // Test si l'entrée fournie est la commande de sortie
            if (!strcmp (EXIT_CMD, user_entry.str))
            {
                fprintf (stdout, "\nExiting\n");
                exit_loop = 1;
            }
        } while (!is_solution (&anagram, user_entry.str) && !exit_loop); /* teste si l'entrée est une solution */
    }


    // Libération de la mémoire
    free_pstring (&user_entry);
    free_anagram (&anagram);
    free_dico (&dico);

    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
