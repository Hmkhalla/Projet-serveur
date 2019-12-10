/*
 * =====================================================================================
 *
 *       Filename:  parrot_server.c
 *
 *    Description:  Un serveur perroquet connecté simple 
 *
 *        Version:  1.0
 *        Created:  13/11/2019 00:34:56
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Guillerme Duvillié (mfreeze), guillerme@duvillie.eu
 *   Organization:  
 *
 * =====================================================================================
 */
#define	_DEFAULT_SOURCE			/* Utilise les implémentation POSIX des fonctions réseaux */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>


#define	TAILLE_BUFFER 1024			/* Taille initiale du buffer de réception */

void *manage_client (void *data);

/*
 * XXX ATTENTION!
 *
 * Le code source présenté ci-dessous utilise des buffers de taille fixe pour faciliter la lecture et
 * se concentrer sur la partie réseau. Ce code ne doit JAMAIS être utilisé tel quel dans une
 * quelconque application!
 *
 * En effet, l'utilisation de buffers de taille fixe le rend particulièrement sensible aux buffers
 * overflow (dépassement de mémoire), dans le meilleur des cas, le programme est instable (risque de
 * segmentation fault) dans le pire des cas, il est vulnérable.
 *
 * Pour une gestion correcte de la mémoire, un autre corrigé vous est fourni.
 */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  main function
 * =====================================================================================
 */
    int
main (int argc, char **argv)
{
    // On définit une structure d'adresse de socket:
    // celle relative au serveur (qui va servir pour le bind, puisqu'on est côté serveur)
    // client)
    struct sockaddr_in adresse_serveur;

    int res;
    int port;

    int local_socket;

    pthread_t new_thread;

    /*-----------------------------------------------------------------------------
     *  Initialisation des structures
     *-----------------------------------------------------------------------------*/
    /* {{{ -------- Initialisation des structures -------- */
   
    memset (&adresse_serveur, 0, sizeof (struct sockaddr_in));

    /* }}} */
    
    /*-----------------------------------------------------------------------------
     *  Processing des paramètres
     *-----------------------------------------------------------------------------*/
    /* {{{ -------- Processing des paramètres -------- */

    // test du nombre de paramètre
    if (argc != 2)
    {
        fprintf (stderr, "parrot_server port\n");
        return EXIT_FAILURE;
    }

    // Port
    /* 
     * La conversion d'une chaîne de caractères en entier peut paraître anodine, mais elle ne l'est
     * pas: on ne peut pas utiliser le retour seul pour détecter une erreur (puisque rien n'indique
     * que la chaîne à convertir ne contient pas cette valeur de retour d'erreur. 
     *
     * Pour pallier à ća (et pour gérer les erreurs en général, on utilise la bibliothèque errno.
     * Cette bibliothèque déclare un entier (errno) qui contient un code standard d'erreur.
     * Attention, même si l'interface est unifiée, il appartient à chacune des fonctions d'affecter
     * ou/non une valeur à errno en cas d'erreur. 
     *
     * Par exemple la fonction atoi (ascii to int) qui permet également une conversion d'une chaîne
     * de caractères vers un entier, n'affecte pas de valeur à errno en cas d'erreur, il est donc
     * impossible de savoir si la conversion s'est faite de manière correcte! C'est pourquoi on
     * utilise la fonction strtol.
     * */

    // Comme chaque fonction peut modifier la valeur de cette variable (c'est une variable globale),
    // on l'initialise à une valeur connue avant l'appel à strtol.
    errno = 0;

    port = strtol (argv[1], NULL, 10);                 /* Conversion en int */

    // On peut ensuite tester si tout s'est bien passé (un coup d'oeil à la page de manuel de strtol
    // nous permet de connaître son fonctionnement en cas d'erreur
    if (errno != 0 && port == 0)
    {
        // perror permet d'afficher un message d'erreur approprié en fonction de la valeur de errno,
        // Elle affiche le préfixe donné en paramètre suivi du message d'erreur.
        perror ("Impossible de convertir le port <%s>");
        return EXIT_FAILURE;
    }

    adresse_serveur.sin_port = htons (port);     /* Conversion représnetation mémoire */

    // Famille d'adresse
    adresse_serveur.sin_family = AF_INET;

    // Adresse IP: on écoute sur toutes les interfaces disponibles
    adresse_serveur.sin_addr.s_addr = htonl (INADDR_ANY);

    // XXX La structure d'adresse de socket relative pour l'écoute sur le serveur est maintenant prête à être utilisée
    
    /* }}} */


    /*-----------------------------------------------------------------------------
     *  Préparation socket local
     *-----------------------------------------------------------------------------*/
    /* {{{ -------- Préparation socket local -------- */

    // 1. Création socket
    local_socket = socket (PF_INET, SOCK_STREAM, 0);
    if (local_socket == -1)
    {
        /* La fonction socket affecte une valeur à errno en cas d'erreur. Pour avoir un message
         * d'erreur plus détaillé, on peut utiliser, à la place de fprintf, la commande perror:
         *
         * perror ("Impossible d'ouvrir le socket: ");
         * */
        fprintf (stderr, "Imposible d'ouvrir le socket\n");
        return EXIT_FAILURE;
    }

    // 2. Lien entre le descripteur de fichier et la structure (obligatoire pour un serveur)
    res = bind (local_socket, (struct sockaddr *) &adresse_serveur, sizeof (struct sockaddr_in));
    if (res == -1)
    {
        /* La fonction bind donne également une valeu à errno, on peut l'utiliser!
         *
         * perror ("Impossible de lier le socket et la structure d'adresse: ");
         * */
        fprintf (stderr, "Impossible de lier le socket et la structure d'adresse.\n");
        close (local_socket);                   /* On ferme le socket avant de quitter */
        return EXIT_FAILURE;
    }

    // XXX Ici le socket local est prêt à être utilisé!
    
    /* }}} */


    /*-----------------------------------------------------------------------------
     *  Connexion
     *-----------------------------------------------------------------------------*/
    /* {{{ -------- Connexion -------- */

    // Du côté serveur, on initialise une écoute active
    res = listen (local_socket, 20);
    if (res == -1)
    {
        // perror ("Impossible de se mettre en écoute: ");
        fprintf (stderr, "Impossible de se mettre en écoute.\n");
        close (local_socket);
        return EXIT_FAILURE;
    }

    /* }}} */

    // Puis on rentre dans la boucle infinie d'acceptation des connexions 
    while (1)
    {
        // Ces variables servent à communiquer avec un client connecté et à récupérer les
        // informations relatives au client
        int socket_client;
        struct sockaddr_in adresse_client;
        socklen_t taille_struct_addr_client;
        // Comme c'est un entier qui va transiter sur le réseau, on utilise un type de données
        // particulier qui garantit la taille en mémoire de ce dernier
        // On accepte la connexion, cette fonction n'étant pas réservée aux socket internet, on doit
        // effectuer un cast sur la structure d'adresse de socket
        socket_client = accept (local_socket, (struct sockaddr *) &adresse_client, &taille_struct_addr_client);
        if (socket_client == -1)
        {
            // perror ("Connexion impossible: ");
            fprintf (stderr, "Connexion impossible.\n");

            // XXX Pas besoin de quitter le programme, ce n'est pas une erreur critique! On passe
            // juste à l'itération suivante
            continue;
        }

        pthread_create (&new_thread, NULL, manage_client, &socket_client);
    }


    close (local_socket);
    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */


void * manage_client (void *data)
{
    char str_buffer[TAILLE_BUFFER];
    char *str_parser;

    int socket_client = *((int *) data);
    int continue_looping = 1;

    int res;
    int longueur_chaine;
    int taille_recue;
    int taille_envoyee;

    uint32_t packet_size;

    while (continue_looping)
    {
        /*-----------------------------------------------------------------------------
         *  Communication
         *-----------------------------------------------------------------------------*/
        /* {{{ -------- Communication -------- */
        //
        // On attend deux messages, un contenant la taille de la chaîne envoyée, l'autre la chaîne
        // en question
        res = recv (socket_client, &packet_size, sizeof (uint32_t), 0);
        if (res == -1)
        {
            // perror ("Erreur à la réception de la taille: ");
            fprintf (stderr, "Erreur à la réception de la taille.\n");
            close (socket_client);
            continue;
        }

        // On convertit l'entier en représentation machine
        longueur_chaine = ntohl (packet_size);

        /*  Si la taille recue est supérieure à TAILLE_BUFFER, on demande à ne recevoir que
         * TAILLE_BUFFER - 1 (pour laisser un octet de libre pour '\0').
         *
         * XXX Ce n'es pas une solution viable, puisque le client enverra tout de même plus de
         * TAILLE_BUFFER - 1! L'information excédentaire serait alors lue lors d'une autre
         * connextion. 
         *
         * On pourrait gérer un tel cas avec une gestion dynamique de la mémoire, en imbriquant
         * plusieurs boucles ou en transférant l'affichage dans la boucle courante (on affiche au
         * fur et à mesure de la réception).
         * */
        // On s'assure que le buffer a une taille suffisante pour recevoir le message
        if (longueur_chaine > TAILLE_BUFFER - 1) /* Ne pas oublier le caractère \0 ! */
        {
            longueur_chaine = TAILLE_BUFFER - 1;
        }

        // On peut enfin lire le message
        for (str_parser = str_buffer, taille_recue = 0; taille_recue < longueur_chaine; )
        {
            res = recv (socket_client, str_parser, longueur_chaine - taille_recue, 0);
            // Dans un premier temps, on gère les erreurs
            if (res == -1)
            {
                // perror ("Impossible de recevoir le message: ");
                fprintf (stderr, "Impossible de recevoir le message.\n");
                close (socket_client);

                // On force la sortie de boucle
                break;
            }
            else if (res == 0)
            {
                printf ("Fermeture socket côté client.\n");
                continue_looping = 0;
                break;
            }

            taille_recue += res;
            str_parser += res;
        }

        str_buffer[taille_recue] = '\0';

        if (taille_recue != longueur_chaine)
        {
            fprintf (stderr, "Réception partielle: %s\n", str_buffer);
        }
        else
        {
            printf ("%s\n", str_buffer);
        }


		//envoie du message !
		
        // On procède ensuite à l'envoi de ce qui a été lu
        longueur_chaine = strlen (str_buffer);
        packet_size = htonl (longueur_chaine);

        res = send (socket_client, &packet_size, sizeof (uint32_t), 0);
        if (res == -1)
        {
            // perror ("Impossible d'envoyer la taille du message: ");
            fprintf (stderr, "Impossible d'envoyer la taille du message.\n");
            close (socket_client);
            continue;
        }

        // Maintenant on peut envoyer la chaîne de caractères
        /* 
         * On veut gérer le cas où le message est très long et nécessite plusieurs messages pour
         * l'envoi. On effectue donc une boucle de send. La fonction send renvoie le nombre d'octets
         * envoyés, on utilise donc cette variable pour modifier str_parser (qui pointe vers le premier
         * caractère du mesage qui n'a pas été envoyé) et pour compter le nombre d'octet déjà envoyés 
         * */
        for (str_parser = str_buffer, taille_envoyee = 0; taille_envoyee < longueur_chaine; )
        {
            res = send (socket_client, str_parser, longueur_chaine - taille_envoyee, 0);
            // Dans un premier temps, on gère les erreurs
            if (res == -1)
            {
                // perror ("Impossible d'envoyer le message: ");
                fprintf (stderr, "Impossible d'envoyer le message.\n");
                close (socket_client);
                continue;
            }

            taille_envoyee += res;
            str_parser += res;
        }
    }
        /* }}} */
    pthread_exit (NULL);
}
