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


int main (int argc, char **argv)
{
    
    struct sockaddr_in adresse_client;
    struct sockaddr_in adresse_serveur;

    int res;
    int port;

    int local_socket;
    uint32_t packet_size;
    int longueur_chaine;

	char * pseudo; 
    char *str_parser;
    int taille_envoyee = 0;

    // Les vriables nécessaires pour la réception d'un message
    char str_buffer[TAILLE_BUFFER];
    int taille_recue = 0;

    // Variable de multiplexage
    fd_set lecture;


    memset (&adresse_client, 0, sizeof (struct sockaddr_in));
    memset (&adresse_serveur, 0, sizeof (struct sockaddr_in));

    if (argc != 4)
    {
        fprintf (stderr, "parrot_client adr_ip port pseudo\n");
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
    errno = 0;

    port = strtol (argv[2], NULL, 10);                
    if (errno != 0 && port == 0)
    {
        perror ("Impossible de convertir le port :");
        return EXIT_FAILURE;
    }
    
    pseudo = argv[3];

    adresse_serveur.sin_port = htons (port);     
    adresse_serveur.sin_family = AF_INET;

  
    // 1. Création socket
    local_socket = socket (PF_INET, SOCK_STREAM, 0);
    if (local_socket == -1)
    {
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
        fprintf (stderr, "Impossible de lier le socket et la structure d'adresse.\n");
        close (local_socket);                   /* On ferme le socket avant de quitter */
        return EXIT_FAILURE;
    }



    /*-----------------------------------------------------------------------------
     *  Connexion
     *-----------------------------------------------------------------------------*/
    /* {{{ -------- Connexion -------- */

    res =  connect (local_socket, (struct sockaddr *) &adresse_serveur, sizeof (struct sockaddr_in));
    if (res == -1)
    {
        fprintf (stderr, "Impossible de se connecter au serveur.\n");
        close (local_socket);
        return EXIT_FAILURE;
    }

	longueur_chaine = strlen (pseudo);
	packet_size = htonl (longueur_chaine);
    res = send (local_socket, &packet_size, sizeof (uint32_t), 0);
    if (res == -1){
		fprintf (stderr, "Impossible d'envoyer la taille du message.\n");
		close (local_socket);
		return EXIT_FAILURE;
	}
	for (str_parser = pseudo, taille_envoyee = 0; taille_envoyee < longueur_chaine; )
	{
		res = send (local_socket, str_parser, longueur_chaine - taille_envoyee, 0);
		if (res == -1){
			fprintf (stderr, "Impossible d'envoyer le message.\n");
			close (local_socket);
			return EXIT_FAILURE;
		}
		taille_envoyee += res;
		str_parser += res;
	}
    
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

        res = select (1 + max (STDIN_FILENO, local_socket), &lecture, NULL, NULL, NULL);
        if (res == -1)
        {
            perror ("Problème de multiplexage: ");
            close (local_socket);
            return EXIT_FAILURE;
        }

        if (FD_ISSET (STDIN_FILENO, &lecture)) 
        {
            fgets (str_buffer, TAILLE_BUFFER, stdin);
            longueur_chaine = strlen (str_buffer);

            if (str_buffer[longueur_chaine - 1] == '\n')
            {
                str_buffer[longueur_chaine - 1] = '\0';
                longueur_chaine--;
            }
            
            if (!strcmp(str_buffer,"!quit")){
				printf("here");
				close (local_socket);
				return EXIT_SUCCESS;
			}

            packet_size = htonl (longueur_chaine);

            res = send (local_socket, &packet_size, sizeof (uint32_t), 0);
            if (res == -1)
            {
                fprintf (stderr, "Impossible d'envoyer la taille du message.\n");
                close (local_socket);
                return EXIT_FAILURE;
            }

            for (str_parser = str_buffer, taille_envoyee = 0; taille_envoyee < longueur_chaine; )
            {
                res = send (local_socket, str_parser, longueur_chaine - taille_envoyee, 0);
                if (res == -1)
                {
                    fprintf (stderr, "Impossible d'envoyer le message.\n");
                    close (local_socket);
                    return EXIT_FAILURE;
                }

                taille_envoyee += res;
                str_parser += res;
            }
        }
        
        if (FD_ISSET (local_socket, &lecture))
        {
            res = recv (local_socket, &packet_size, sizeof (uint32_t), 0);
            if (res == -1)
            {
                fprintf (stderr, "Erreur à la réception de la taille.\n");
                close (local_socket);
                return EXIT_FAILURE;
            }
            longueur_chaine = ntohl (packet_size);
            for (str_parser = str_buffer, taille_recue = 0; taille_recue < longueur_chaine; )
            {
                res = recv (local_socket, str_parser, longueur_chaine - taille_recue, 0);
                if (res == -1)
                {
                    fprintf (stderr, "Impossible de recevoir le message.\n");
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
            if (taille_recue != longueur_chaine)
            {
                fprintf (stderr, "Réception partielle: %s\n", str_buffer);
            }
            else
            {
                printf ("Message recu: %s\n", str_buffer);
            }
        }
    }

    close (local_socket);
    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
