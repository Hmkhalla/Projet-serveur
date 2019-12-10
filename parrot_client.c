/*
 * =====================================================================================
 *
 *       Filename:  parrot_client.c
 *
 *    Description:  Un client perroquet simple:
 *                      se connecte
 *                      envoie un message
 *                      se déconnecte
 *
 *        Version:  1.0
 *        Created:  12/11/2019 22:27:43
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Guillerme Duvillié (mfreeze), guillerme@duvillie.eu
 *   Organization:  
 *
 * =====================================================================================
 */
#define	_DEFAULT_SOURCE			    /* Utilise les implémentation POSIX des fonctions réseaux */
#define	TAILLE_BUFFER  1024			/* Taille initiale du buffer */

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

#define max(x,y) ( x < y ? y : x )              /* Instruction ternaire pour déterminer le maximum entre deux nombres */
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
    // On définit deux structures d'adresse de socket (une pour le client local qui sera utilisée
    // dans le bind, une autre pour les informations du serveur qui seront données par
    // l'utilisateur)
    struct sockaddr_in adresse_client;
    struct sockaddr_in adresse_serveur;

    int res;
    int port;

    int local_socket;
    // Comme c'est un entier qui va transiter sur le réseau, on utilise un type de données
    // particulier qui garantit la taille en mémoire de ce dernier
    uint32_t packet_size;
    int longueur_chaine;

    // Cette variable sert dans le cas où l'envoi (utilisation de send) nécessite plusieurs appels
    char *str_parser;
    int taille_envoyee = 0;

    // Les vriables nécessaires pour la réception d'un message
    char str_buffer[TAILLE_BUFFER];
    int taille_recue = 0;

    // Variable de multiplexage
    fd_set lecture;

    /*-----------------------------------------------------------------------------
     *  Initialisation des structures
     *-----------------------------------------------------------------------------*/
    /* {{{ -------- Initialisation des structures -------- */
   
    memset (&adresse_client, 0, sizeof (struct sockaddr_in));
    memset (&adresse_serveur, 0, sizeof (struct sockaddr_in));

    /* }}} */
    
    /*-----------------------------------------------------------------------------
     *  Processing des paramètres
     *-----------------------------------------------------------------------------*/
    /* {{{ -------- Processing des paramètres -------- */

    // test du nombre de paramètre
    if (argc != 3)
    {
        fprintf (stderr, "parrot_client adr_ip port mot\n");
        return EXIT_FAILURE;
    }

    // Adresse ip
    res = inet_aton (argv[1], &adresse_serveur.sin_addr);
    if (!res)                                   /* inet_aton retourne 0 en cas d'erreur */
    {
        fprintf (stderr, "Impossible de convertir l'adresse <%s>\n", argv[1]);
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

    port = strtol (argv[2], NULL, 10);                 /* Conversion en int */

    // On peut ensuite tester si tout s'est bien passé (un coup d'oeil à la page de manuel de strtol
    // nous permet de connaître son fonctionnement en cas d'erreur
    if (errno != 0 && port == 0)
    {
        // perror permet d'afficher un message d'erreur approprié en fonction de la valeur de errno,
        // Elle affiche le préfixe donné en paramètre suivi du message d'erreur.
        perror ("Impossible de convertir le port :");
        return EXIT_FAILURE;
    }

    adresse_serveur.sin_port = htons (port);     /* Conversion représentation mémoire */

    // Famille d'adresse
    adresse_serveur.sin_family = AF_INET;

    // XXX La structure d'adresse de socket relative au serveur est maintenant prête à être utilisée
    
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

    // 2. Préparation de la structure d'adresse de socket locale
    adresse_client.sin_family = AF_INET;
    adresse_client.sin_port = htons (0);            /* On laisse le choix du port à l'OS */
    adresse_client.sin_addr.s_addr = htonl (INADDR_ANY);

    // 3. Lien entre le descripteur de fichier et la structure (optionnel pour le client mais
    // recommandé)
    res = bind (local_socket, (struct sockaddr *) &adresse_client, sizeof (struct sockaddr_in));
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

    // On effectue la demande de connexion en utilisant le socket lié localement et la structure
    // d'adresse contenant les informations serveur. Comme pour un coup de téléphone, on utilise le
    // téléphone fixe personnel mais avec le numéro de téléphone de l'interlocuteur
    res =  connect (local_socket, (struct sockaddr *) &adresse_serveur, sizeof (struct sockaddr_in));
    if (res == -1)
    {
        // perror ("Impossible de se connecter au serveur: ");
        fprintf (stderr, "Impossible de se connecter au serveur.\n");
        close (local_socket);
        return EXIT_FAILURE;
    }

    // XXX Ici la connexion entre le client et le serveur est établie

    /* }}} */

    /*-----------------------------------------------------------------------------
     *  Communication
     *-----------------------------------------------------------------------------*/
    /* {{{ -------- Communication -------- */

    while (1)
    {
        /*-----------------------------------------------------------------------------
         *  Multiplexage
         *-----------------------------------------------------------------------------*/
        /* {{{ -------- Multiplexage -------- */
        FD_ZERO (&lecture);
        FD_SET (STDIN_FILENO, &lecture);            /* l'entrée standard */
        FD_SET (local_socket, &lecture);            /* le socket connecté au serveur */
        /* }}} */

        res = select (1 + max (STDIN_FILENO, local_socket), &lecture, NULL, NULL, NULL);
        if (res == -1)
        {
            perror ("Problème de multiplexage: ");
            close (local_socket);
            return EXIT_FAILURE;
        }

        if (FD_ISSET (STDIN_FILENO, &lecture))  /* Si c'est le clavier, on lit le message et on l'envoie */
        {
            fgets (str_buffer, TAILLE_BUFFER, stdin);

            // On convertit l'entier en représentation réseau
            longueur_chaine = strlen (str_buffer);

            /* 
             * On enlève le \n au besoin
             * */
            if (str_buffer[longueur_chaine - 1] == '\n')
            {
                str_buffer[longueur_chaine - 1] = '\0';
                longueur_chaine--;
            }

            packet_size = htonl (longueur_chaine);

            // Et on l'envoie au serveur
            res = send (local_socket, &packet_size, sizeof (uint32_t), 0);
            if (res == -1)
            {
                // perror ("Impossible d'envoyer la taille du message: ");
                fprintf (stderr, "Impossible d'envoyer la taille du message.\n");
                /* Ici fermer le socket est primordial, en effet, le serveur attend un message qui
                 * n'arrivera jamais, la fermeture du socket entraîne une notification à l'hôte connecté
                 * évitant ainsi que ce dernier n'attende pour rien */
                close (local_socket);
                return EXIT_FAILURE;
            }

            // Maintenant on peut envoyer la chaîne de caractères

            /* 
             * Il faut gérer le cas où le message ne part pas en une seul fois, on utilise donc une boucle
             * pour appeler send autant de fois que nécessaire. Le pointeur str_parser permet de reprendre
             * là où le précédent appel à send sést arrêté.
             * */
            for (str_parser = str_buffer, taille_envoyee = 0; taille_envoyee < longueur_chaine; )
            {
                res = send (local_socket, str_parser, longueur_chaine - taille_envoyee, 0);
                // Dans un premier temps, on gère les erreurs
                if (res == -1)
                {
                    // perror ("Impossible d'envoyer le message: ");
                    fprintf (stderr, "Impossible d'envoyer le message.\n");
                    close (local_socket);
                    return EXIT_FAILURE;
                }

                taille_envoyee += res;                  /* On ajoute le nombre d'octets de l'envoi courant au nombre d'octets envoyés */
                str_parser += res;                      /* De la même manière on décale str_parser de manière à reprendre là où send s'est arrêté */
            }
        }
        
        if (FD_ISSET (local_socket, &lecture))  /* Si c'est le socket, on lit et on affiche le message recu */
        {
            // On attend deux messages, un contenant la taille de la chaîne envoyée, l'autre la chaîne
            // en question
            res = recv (local_socket, &packet_size, sizeof (uint32_t), 0);
            if (res == -1)
            {
                // perror ("Erreur à la réception de la taille: ");
                fprintf (stderr, "Erreur à la réception de la taille.\n");
                close (local_socket);
                return EXIT_FAILURE;
            }

            // On convertit l'entier en représentation machine
            longueur_chaine = ntohl (packet_size);

            // On peut lire le message
            for (str_parser = str_buffer, taille_recue = 0; taille_recue < longueur_chaine; )
            {
                res = recv (local_socket, str_parser, longueur_chaine - taille_recue, 0);
                // Dans un premier temps, on gère les erreurs
                if (res == -1)
                {
                    // perror ("Impossible de recevoir le message: ");
                    fprintf (stderr, "Impossible de recevoir le message.\n");

                    // S'il y a une erreur, on se contente d'une réception partielle et on sort de la boucle
                    break;
                }
                else if (res == 0)
                {
                    printf ("Fermeture socket côté serveur.\n");

                    break;
                }

                taille_recue += res;
                str_parser += res;
            }

            str_buffer[taille_recue] = '\0';

            // En cas de réception partielle, on informe l'utilisateur
            if (taille_recue != longueur_chaine)
            {
                fprintf (stderr, "Réception partielle: %s\n", str_buffer);
            }
            // Sinon on affiche le message recu
            else
            {
                printf ("Message recu: %s\n", str_buffer);
            }
        }
    }
    /* }}} */

    close (local_socket);
    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
