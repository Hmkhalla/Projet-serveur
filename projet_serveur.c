//MKHALLATI Hassan 000459680
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
#include "lib.h"
#include "anagram.h"
#include "personal_strings.h"


#define	INIT_TAILLE_BUFFER 1024			/* Taille initiale du buffer de réception */

void *manage_client (void *data);

char * welcome_message;
dict_t dico;
structArray *players;
int local_socket;

typedef struct  {
    int socket_client;
    pthread_mutex_t mutex;
}arg_struct;

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  main function
 * =====================================================================================
 */
int main (int argc, char **argv)
{
    
    struct sockaddr_in adresse_serveur;

    int res;
    int port;
	
	
    pthread_t new_thread;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    
    
    memset (&adresse_serveur, 0, sizeof (struct sockaddr_in));
    // test du nombre de paramètre
    if (argc != 4)
    {
        fprintf (stderr, "maranga_srv port path_dict welcome\n");
        return EXIT_FAILURE;
    }
	welcome_message = argv[3];
	
    errno = 0;
    port = strtol (argv[1], NULL, 10);                 /* Conversion en int */
    if (errno != 0 && port == 0)
    {
        perror ("Impossible de convertir le port: ");
        return EXIT_FAILURE;
    }

    // Adresse serveur
    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_addr.s_addr = htonl (INADDR_ANY);
    adresse_serveur.sin_port = htons (port);     /* Conversion représnetation mémoire */

    // 1. Création socket serveur
    local_socket = socket (PF_INET, SOCK_STREAM, 0);
    if (local_socket == -1)
    {
        perror ("Impossible d'ouvrir le socket: ");
        return EXIT_FAILURE;
    }

    // 2. Lien entre le descripteur de fichier et la structure (obligatoire pour un serveur)
    res = bind (local_socket, (struct sockaddr *) &adresse_serveur, sizeof (struct sockaddr_in));
    if (res == -1)
    {
        perror ("Impossible de lier le socket et la structure d'adresse: ");
        close (local_socket);  
        return EXIT_FAILURE;
    }

/*------------------------------------------------CONNEXION-------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------*/
	
    res = listen (local_socket, 20);
    if (res == -1)
    {
        perror ("Impossible de se mettre en écoute: ");
        close (local_socket);
        return EXIT_FAILURE;
    }

/*---------------------------------INIT des données pour le jeu anagram-------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------*/

	players = init_memory(); //initialisation de la structure de données relatives aux scores des joueurs.
	if (!players)
    {
        fprintf (stderr, "Erreur initialisation Tableau: ");
        close (local_socket);
        return EXIT_FAILURE;
    }
	res = init_dico (&dico, argv[2]);
    if (res == -1)
    {
        perror ("Erreur initialisation dictionnaire: ");
        delete_table(players);
        close (local_socket);
        return EXIT_FAILURE;
    }

/*---------------------------------Boucle d'acceptation de connexion----------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------*/
	
    while (1)
    {
        // informations relatives au client
        int socket_client;
        struct sockaddr_in adresse_client;
        socklen_t taille_struct_addr_client;
        socket_client = accept (local_socket, (struct sockaddr *) &adresse_client, &taille_struct_addr_client);
        if (socket_client == -1)
        {
            perror ("Connexion impossible: ");
            continue;
        }
		
		arg_struct data;
		data.socket_client=socket_client;
		data.mutex = mutex;
        pthread_create (&new_thread, NULL, manage_client, &data);
    }


    close (local_socket);
    free_dico (&dico);
    delete_table(players);
    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */


void * manage_client (void *data)
{
	
    char * str_buffer;
    char * str_parser;
	char congrats[INIT_TAILLE_BUFFER];
    //Une gestion dynamique de la mémoire n'est pas nécessaire, vu que le message de récompense est connu. 
	char * pseudo;
	int  indexPlayer;
	anagram_t anagram;
	
    int res;
    int longueur_chaine = 0;
    int taille_recue = 0;
    int taille_envoyee = 0;
    int taille_courante = 0;
    uint32_t packet_size;
    
    arg_struct arg = *((arg_struct*)data);
    int socket_client = arg.socket_client;
    pthread_mutex_t mutex = arg.mutex;
    

/*------------------------------------------------recevoir pseudo-------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------*/

	res = recv (socket_client, &packet_size, sizeof (uint32_t), 0);
	if (res == -1)
    {
		perror ("Erreur à la réception de la taille: ");
		close (socket_client);
		pthread_exit(NULL);
	}
	longueur_chaine = ntohl (packet_size);
	pseudo	= (char *) malloc (sizeof(char) * (longueur_chaine + 1));
	if (!pseudo) {
	    perror ("Intialisation mémoire buffer de réception: ");
		close (socket_client);
		pthread_exit(NULL);
	}
	for (str_parser = pseudo, taille_recue = 0; taille_recue < longueur_chaine; )
	{
		res = recv (socket_client, str_parser, longueur_chaine - taille_recue, 0);
		if (res == -1)
		{
			perror ("Impossible de recevoir le message: ");
			free(pseudo);
			close (socket_client);
			pthread_exit(NULL);
		}
		else if (res == 0)
		{
			printf ("Fermeture socket côté client.\n");
			free(pseudo);
			close (socket_client);
			pthread_exit(NULL);
		}
		taille_recue += res;
		str_parser += res;
	}
	pseudo[taille_recue] = '\0';
	if (taille_recue != longueur_chaine)
	{//Le pseudo étant l'ID d'un joueur, sa réception doit être totale !
		fprintf (stderr, "Réception partielle du pseudo :<%s>\n", pseudo);
		free(pseudo);
		close (socket_client);
		pthread_exit(NULL);
	}
	else
	{
		printf( "Connexion du client : %s\n", pseudo);
	}
	
	//Il faut vérouiller le tableau afin de pouvoir écrire en toute sécurité.
	res = pthread_mutex_lock (&mutex);
    if (res != 0)
    {
        perror ("Impossible de verrouiller la zone mémoire: ");
		close (socket_client);
		free(pseudo);
		pthread_exit(NULL);
    }
	res = add_player(players, pseudo, &indexPlayer);
	if (res==-1){
		fprintf (stderr, "Erreur ajout du joueur au tableau. Mémoire corrompue\n");
		free(pseudo);
		free_dico (&dico);
		close (socket_client);
		close (local_socket);
		delete_table(players);
		exit(EXIT_FAILURE);
	}
	pthread_mutex_unlock (&mutex);
	
	/*S'il y a une erreur, il ne sera plus nécessaire de libérer pseudo car une shallow copy a été faite.*/

/*------------------------------------------------envoie message d'acceuil----------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------*/

	longueur_chaine = strlen (welcome_message);
	packet_size = htonl (longueur_chaine);
	res = send (socket_client, &packet_size, sizeof (uint32_t), 0);
	if (res == -1)
	{
		perror ("Impossible d'envoyer la taille du message: ");
		close (socket_client);
		pthread_exit(NULL);
	}
	for (str_parser = welcome_message, taille_envoyee = 0; taille_envoyee < longueur_chaine; )
	{
		res = send (socket_client, str_parser, longueur_chaine - taille_envoyee, 0);
		if (res == -1)
		{
			perror ("Impossible d'envoyer le message: ");
			close (socket_client);
			pthread_exit(NULL);
		}
	
		taille_envoyee += res;
		str_parser += res;
	}
	
/*----------------------------------------------------INIT VARIABLE DE COM----------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------*/

	str_buffer	= (char *) malloc (sizeof(char) * INIT_TAILLE_BUFFER);
    if (!str_buffer) {
	    perror ("Intialisation mémoire buffer de réception");
        close (socket_client);
        pthread_exit(NULL);
    }
	taille_courante=INIT_TAILLE_BUFFER;

/*----------------------------------------------------INIT ANAGRAME-----------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------*/
    init_anagram (&anagram);
	
/*----------------------------------------------------boucle de jeu-----------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------*/
	
    while (1)
    {
		/*-----------------------------------Génération anagrame et envoie anagramme----------------------------------*/
	/*----------------------------------------------------------------------------------------------------------------*/
		res = new_anagram (&anagram, &dico);
		if (res == -1)
		{
			perror ("Création d'un nouvel anagramme: ");
			free_anagram (&anagram);
			free(str_buffer);
			close (socket_client);
			pthread_exit(NULL);
		}
        longueur_chaine = strlen (anagram.mot_courant.str);
        packet_size = htonl (longueur_chaine);
        res = send (socket_client, &packet_size, sizeof (uint32_t), 0);
        if (res == -1)
        {
            perror ("Impossible d'envoyer la taille du message: ");
			free_anagram (&anagram);
			free(str_buffer);
			close (socket_client);
			pthread_exit(NULL);
        }
        for (str_parser = anagram.mot_courant.str, taille_envoyee = 0; taille_envoyee < longueur_chaine; )
        {
            res = send (socket_client, str_parser, longueur_chaine - taille_envoyee, 0);
            if (res == -1)
            {
                perror ("Impossible d'envoyer le message: ");
				free_anagram (&anagram);
				free(str_buffer);
				close (socket_client);
				pthread_exit(NULL);
            }

            taille_envoyee += res;
            str_parser += res;
        }
		/*--------------------------------------------réception de la réponse-------------------------------------*/
		/*--------------------------------------------------------------------------------------------------------*/
		while (!is_solution (&anagram, str_buffer)) {
	        res = recv (socket_client, &packet_size, sizeof (uint32_t), 0);
	        if (res == -1)
	        {
	            perror ("Erreur à la réception de la taille: ");
	            free_anagram (&anagram);
				free(str_buffer);
				close (socket_client);
				pthread_exit(NULL);
	        }
	        longueur_chaine = ntohl (packet_size);
			if (taille_courante <= longueur_chaine) /* Ne pas oublier le caractère \0 ! */
			{
				str_buffer	= realloc (str_buffer, sizeof(char) * (longueur_chaine + 1));
				if (!str_buffer) {
					fprintf (stderr, "Réallocation impossible\n");
					free_anagram (&anagram);
					free(str_buffer);
					close (socket_client);
					pthread_exit(NULL);
				}
				taille_courante=longueur_chaine + 1;
			}
	        for (str_parser = str_buffer, taille_recue = 0; taille_recue < longueur_chaine; )
	        {
	            res = recv (socket_client, str_parser, longueur_chaine - taille_recue, 0);
	            if (res == -1)
	            {
					perror ("Impossible de recevoir le message: ");
	                free_anagram (&anagram);
					free(str_buffer);
					close (socket_client);
					pthread_exit(NULL);
	            }
	            else if (res == 0)
	            {
	                printf ("Fermeture socket côté client.\n");
	                free_anagram (&anagram);
					free(str_buffer);
					close (socket_client);
					pthread_exit(NULL);
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
		}
		/*--------------------------------------------gestion  de la réponse--------------------------------------*/
		/*--------------------------------------------------------------------------------------------------------*/
		int score = strlen(str_buffer);
		res = pthread_mutex_lock (&mutex);
	    if (res != 0)
	    {
	        perror ("Impossible de verrouiller la zone mémoire: ");
	        free_anagram (&anagram);
			free(str_buffer);
			close (socket_client);
			pthread_exit(NULL);
	    }
	    players->table[indexPlayer].score+=score;
	    int total = players->table[indexPlayer].score;
		pthread_mutex_unlock (&mutex);
		
		res = snprintf(congrats, sizeof(congrats), "Bravo, vous avez gagné %d points (total: %d)", score, total);
		if (res<0) continue; 
		longueur_chaine = strlen (congrats);
        packet_size = htonl (longueur_chaine);
        res = send (socket_client, &packet_size, sizeof (uint32_t), 0);
        if (res == -1)
        {
            perror ("Impossible d'envoyer la taille du message: ");
			free_anagram (&anagram);
			free(str_buffer);
			close (socket_client);
			pthread_exit(NULL);
        }
        for (str_parser = congrats, taille_envoyee = 0; taille_envoyee < longueur_chaine; )
        {
            res = send (socket_client, str_parser, longueur_chaine - taille_envoyee, 0);
            if (res == -1)
            {
                perror ("Impossible d'envoyer le message: ");
				free_anagram (&anagram);
				free(str_buffer);
				close (socket_client);
				pthread_exit(NULL);
            }
            taille_envoyee += res;
            str_parser += res;
        }
    }
}
