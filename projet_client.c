//MKHALLATI Hassan 000459680
#define	_DEFAULT_SOURCE			    /* Utilise les implémentation POSIX des fonctions réseaux */
#define	INIT_TAILLE_BUFFER  1024			/* Taille initiale du buffer */
#define EXIT_CMD  "!quit"

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
#include "personal_strings.h"

#define max(x,y) ( x < y ? y : x )

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  main function
 * =====================================================================================
 */
int main (int argc, char **argv)
{
    
    struct sockaddr_in adresse_client;
    struct sockaddr_in adresse_serveur;
    int local_socket;

    int res;
    int port;

    
    uint32_t packet_size;
    int longueur_chaine;
    int taille_courante = 0;
    int taille_envoyee = 0;
    int taille_recue = 0;

	pstring_t user_entry;
	char * pseudo;
    char * str_parser;
    char *str_buffer;

    // Variable de multiplexage
    fd_set lecture;


    memset (&adresse_client, 0, sizeof (struct sockaddr_in));
    memset (&adresse_serveur, 0, sizeof (struct sockaddr_in));

    if (argc != 4)
    {
        fprintf (stderr, "maranga_cli adr_ip port pseudo\n");
        return EXIT_FAILURE;
    }
    pseudo = argv[3];
    
    res = inet_aton (argv[1], &adresse_serveur.sin_addr); //Adresse serveur 
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
        perror ("Impossible de convertir le port: ");
        return EXIT_FAILURE;
    }
    
    adresse_serveur.sin_port = htons (port);     
    adresse_serveur.sin_family = AF_INET;

  
    // 1. Création socket
    local_socket = socket (PF_INET, SOCK_STREAM, 0);
    if (local_socket == -1)
    {
        perror ("Impossible d'ouvrir le socket: ");
        return EXIT_FAILURE;
    }

    // 2. Préparation de la structure d'adresse de socket locale
    adresse_client.sin_family = AF_INET;
    adresse_client.sin_port = htons (0);            /* On laisse le choix du port à l'OS */
    adresse_client.sin_addr.s_addr = htonl (INADDR_ANY);

    // 3. Lien entre le descripteur de fichier et la structure
    res = bind (local_socket, (struct sockaddr *) &adresse_client, sizeof (struct sockaddr_in));
    if (res == -1)
    {
        perror ("Impossible de lier le socket et la structure d'adresse: ");
        close (local_socket);  
        return EXIT_FAILURE;
    }



/*------------------------------------------------CONNEXION-------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------*/

    res =  connect (local_socket, (struct sockaddr *) &adresse_serveur, sizeof (struct sockaddr_in));
    if (res == -1)
    {
        fprintf (stderr, "Impossible de se connecter au serveur.\n");
        close (local_socket);
        return EXIT_FAILURE;
    }

/*------------------------------------------------Envoie pseudo---------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------*/
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

/*----------------------------------------INIT variable de communication------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------*/   
    str_buffer	= (char *) malloc (sizeof(char) * INIT_TAILLE_BUFFER);
    if (!str_buffer) {
	    perror ("Intialisation mémoire buffer de réception");
        close (local_socket);
        return EXIT_FAILURE;
    }
	taille_courante=INIT_TAILLE_BUFFER;
	
	user_entry = empty_pstring ();


/*------------------------------------------------Communication---------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------*/

    while (1)
    {
        /*------------------------------------------------Multiplexage----------------------------------------------------*/
		/*----------------------------------------------------------------------------------------------------------------*/
        FD_ZERO (&lecture);
        FD_SET (STDIN_FILENO, &lecture);            /* l'entrée standard */
        FD_SET (local_socket, &lecture);            /* le socket connecté au serveur */

        res = select (1 + max (STDIN_FILENO, local_socket), &lecture, NULL, NULL, NULL);
        if (res == -1)
        {
            perror ("Problème de multiplexage: ");
            close (local_socket);
            free (str_buffer);
            free_pstring (&user_entry);
            return EXIT_FAILURE;
        }

        if (FD_ISSET (STDIN_FILENO, &lecture)) 
        {/*-------------------lecture clavier ------------------*/
			res = secure_read_line (&user_entry, stdin);
            if (res == -1)
            {
                perror ("Lecture entrée client: ");
                free_pstring (&user_entry);
                free (str_buffer);
				close (local_socket);
                return EXIT_FAILURE;
            }
			
            if (!strcmp (EXIT_CMD, user_entry.str))
            {/*-------------------Commande de sortie utilisateur--------------*/
                fprintf (stdout, "\nExiting\n");
                free_pstring (&user_entry);
                close (local_socket);
                free (str_buffer);
                return EXIT_SUCCESS;
            }
            
            longueur_chaine = user_entry.len;
            packet_size = htonl (longueur_chaine);

            res = send (local_socket, &packet_size, sizeof (uint32_t), 0);
            if (res == -1)
            {
                fprintf (stderr, "Impossible d'envoyer la taille du message.\n");
                free_pstring (&user_entry);
                close (local_socket);
                free (str_buffer);
                return EXIT_FAILURE;
            }
			
            for (str_parser = user_entry.str, taille_envoyee = 0; taille_envoyee < longueur_chaine; )
            {
                res = send (local_socket, str_parser, longueur_chaine - taille_envoyee, 0);
                if (res == -1)
                {
                    fprintf (stderr, "Impossible d'envoyer le message.\n");
                    close (local_socket);
                    free (str_buffer);
                    free_pstring (&user_entry);
                    return EXIT_FAILURE;
                }
                taille_envoyee += res;
                str_parser += res;
            }
        }
        
        
        if (FD_ISSET (local_socket, &lecture))
        {/*---------------------------------------lecture serveur----------------------------*/
			res = recv (local_socket, &packet_size, sizeof (uint32_t), 0);
			if (res == -1)
			{
				fprintf (stderr, "Erreur à la réception de la taille.\n");
				close (local_socket);
				free (str_buffer);
				free_pstring (&user_entry);
				return EXIT_FAILURE;
			}
			longueur_chaine = ntohl (packet_size);
			if (taille_courante <= longueur_chaine) /* Ne pas oublier le caractère \0 ! */
			{
				char *tmp_buf;
				tmp_buf	= realloc (str_buffer, sizeof(char) * (longueur_chaine + 1));
				if (!tmp_buf) {
					fprintf (stderr, "Réallocation impossible\n");
					free (str_buffer);
					free_pstring (&user_entry);
					close (local_socket);
					return EXIT_FAILURE;
				}
				str_buffer = tmp_buf;
				taille_courante=longueur_chaine + 1;
			}
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
					free (str_buffer);
					free_pstring (&user_entry);
					close (local_socket);
					return EXIT_FAILURE;
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
	free (str_buffer);
	free_pstring (&user_entry);
    close (local_socket);
    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
