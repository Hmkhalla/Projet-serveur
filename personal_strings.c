//MKHALLATI Hassan 000459680
#include "personal_strings.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  empty_pstring
 *  Description:  Retourne une chaine vide, avec tous les champs initialisés à 0 ou 
 *                  NULL
 *   Parameters:  
 *       Return:  une chaîne vide
 * =====================================================================================
 */
/* --------- empty_pstring --------- {{{ */
    pstring_t
empty_pstring ()
{
    pstring_t pstr = {.str = NULL, .size = 0, .len = 0};

    return pstr;
}		/* -----  end of function empty_pstring  ----- */
/* }}} */


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  init_pstring
 *  Description:  Initialise une structure pstring. Le tampon est alloué dynamiquement
 *                  à la taille spécifiée par le paramètre init_size et initialisée à 0.
 *                  En cas d'erreur, le tampon est mis à NULL et les autres champs mis
 *                  à 0.
 *   Parameters:  pstring *pstr: un pointeur sur la structure à initialiser
 *                int init_size: la taille demadée pour le tampon
 *       Return:  -1 en cas d'erreur, 0 sinon.
 * =====================================================================================
 */
/* --------- init_pstring --------- {{{ */
    int
init_pstring (pstring_t *pstr, int init_size)
{
    /*
     * La fonction calloc remplit automatiquement le tampon avec des 0
     */
    pstr->str = (char *) calloc (init_size, sizeof (char));
    if (! pstr->str)
    {
        pstr->len = 0;
        pstr->size = 0;
        return -1;
    }

    pstr->len = 0;
    pstr->size = init_size;

    return 0;
}		/* -----  end of function init_pstring  ----- */
/* }}} */


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  free_pstring
 *  Description:  Libère l'espace alloué pour une structure pstring_t, tous les champs
 *                  sont mis à zéro (sauf str mis à NULL)
 *   Parameters:  pstring_t *pstr: un pointeur sur la structure
 *       Return:  
 * =====================================================================================
 */
/* --------- free_pstring --------- {{{ */
    void
free_pstring (pstring_t *pstr)
{
    if (pstr->str)
    {
        free (pstr->str);
        pstr->str = NULL;
    }

    pstr->len = 0;
    pstr->size = 0;
}		/* -----  end of function free_pstring  ----- */
/* }}} */



/*
 * ===  FUNCTION  ======================================================================
 *         Name:  secure_read_line
 *  Description:  Lit une ligne complète dans le flux passé en paramètre, réalloue la 
 *                  mémoire du tampon de la structure au besoin. Le contenu de la chaîne
 *                  est écrasé. Si le tampon de chaîne est égal à NULL ou si le taille
 *                  allouée est 0, on procède à l'initialisation de la chaîne avec un 
 *                  buffer de INIT_STR_BUFFER_SIZE octets. Cette fonction supprime
 *                  également les caractères \n en fin de chaîne
 *   Parameters:  pstring_t *pstr: un pointeur sur le structure de chaîne
 *                FILE *stream: le flux dans lequel la chaîne doit être lue
 *       Return:  1 en cas de succès, 0 si aucun caractère n'est disponible en lecture
 *                  ou en cas d'erreur.
 * =====================================================================================
 */
/* --------- secure_read_line --------- {{{ */
    int
secure_read_line (pstring_t *pstr, FILE *stream)
{
    char *parser;                               /* un parser utilisé pour remplir le tampon séquentiellement */
    char *ret_s;
    int ret_val;
    int remaining_size;
    int continue_looping = 1;

    if (!pstr->str || !pstr->size) 
    {
        ret_val = init_pstring (pstr, INIT_STR_BUFFER_SIZE);
        if (ret_val == -1)
        {
            return ret_val;
        }
    }

    pstr->len = 0;
    parser = pstr->str;
    remaining_size = pstr->size;
    
    do
    {
        ret_s = fgets (parser, remaining_size, stream);
        /*
         * Si ret_s est différent de NULL, alors des caractères ont été lus, on cherche donc le
         * caractère de fin de ligne et/ou le caractère de fin de chaîne pour savoir si la lecture a 
         * été complète
         */
        if (ret_s)
        {
            int cur_len = strlen (parser);
            pstr->len += cur_len;
            /*
             * Si la longueur de la chaîne est égale à la taille demandée (moins le caractère de fin
             * de chaîne), on vérifie si le dernier caractère est une fin de ligne. Si oui, la
             * lecture est complète, si non, on agrandit la taille du tampon et on recommence à
             * lire.
             */
            if (cur_len < remaining_size - 1 || parser[remaining_size - 2] == '\n')
            {
                continue_looping = 0;
            }
            else
            {
                char *tmp_buffer = (char *) realloc (pstr->str, sizeof (char) * pstr->size * 2);
                if (!tmp_buffer)
                {
                    return 0;                          /* errno est initialisé grâce à realloc */
                }

                parser = tmp_buffer + pstr->len;
                remaining_size = pstr->size + 1;
                pstr->str = tmp_buffer;
                pstr->size *= 2;
            }
        }
        else
        {
            continue_looping = 0;
        }
    } while (continue_looping);				/* -----  end do-while  ----- */

    
    if (!ret_s)
    {
        return 0;
    }

    parser = pstr->str + (pstr->len - 1);

    if (*parser == '\n')
    {
        *parser = '\0';
        pstr->len--;
    }
    return 1;
}		/* -----  end of function secure_read_line  ----- */
/* }}} */



/*
 * ===  FUNCTION  ======================================================================
 *         Name:  copy_pstring
 *  Description:  Copie le contenu du tampon d'une pstring dans une autre pstring. Si 
 *                  la pstring destinataire ne dispose pas d'un espace suffisant, on 
 *                  réalloue un segment mémoire de taille égale au tampon de la pstring
 *                  source
 *   Parameters:  pstring_t *dest: la pstring dans laquelle sera copiée le tampon
 *                const pstring_t *src: la pstring source
 *       Return:  0 en cas de succès, -1 en cas d'erreur
 * =====================================================================================
 */
/* --------- copy_pstring --------- {{{ */
    int
copy_pstring (pstring_t *dest, const pstring_t *src)
{
    if (!dest->str)
    {
        dest->str = (char *) calloc (sizeof (char), src->size);
        if (!dest->str)
        {
            return -1;
        }

        dest->size = src->size;
        dest->len = 0;
    }
    else if (dest->size <= src->len)
    {
        char *tmp_buf = (char *) realloc (dest->str, sizeof (char) * src->size);
        if (!tmp_buf)
        {
            return -1;
        }

        dest->str = tmp_buf;
        dest->size = src->size;
        dest->len = 0;
    }

    strncpy (dest->str, src->str, dest->size);
    dest->len = src->len;

    return 0;
}		/* -----  end of function copy_pstring  ----- */
/* }}} */



/*
 * ===  FUNCTION  ======================================================================
 *         Name:  ncopy_string
 *  Description:  Copie jusque len caractères du contenu de la chaîne de caractères src 
 *                  dans le tampon de la pstring dest. La fonction s'assure également de 
 *                  la présence d'un caractère de fin de chaîne
 *   Parameters:  pstring_t *dest: la pstring destinataire
 *                const char *src: la chaîne source
 *                int len: le nombre de caractères max à copier
 *       Return:  -1 en cas d'erreur, 0 sinon
 * =====================================================================================
 */
/* --------- ncopy_string --------- {{{ */
    int
ncopy_string (pstring_t *dest, const char *src, int len)
{
    if (!dest->str)
    {
        dest->str = (char *) calloc (sizeof (char), len + 1);
        if (!dest->str)
        {
            return -1;
        }

        dest->size = len + 1;
        dest->len = 0;
    }
    else if (dest->size <= len)
    {
        char *tmp_buf = (char *) realloc (dest->str, sizeof(char) * (len + 1));
        if (!tmp_buf) {
            return -1;
        }

        dest->str = tmp_buf;
        dest->size = len + 1;
        dest->len = 0;
    }

    strncpy (dest->str, src, len);
    dest->len = len;
    dest->str[len] = '\0';

    return 0;
}		/* -----  end of function ncopy_string  ----- */
/* }}} */


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  copy_string
 *  Description:  Copie le contenu complet d'une chaîne de caractère src (qui doit être
 *                  terminée par un caractère de fin de chaîne) dans le tampon de la 
 *                  pstring dest
 *   Parameters:  pstring_t *dest: la pstring qui va recevoir la chaîne
 *                const char *src: la chaîne à copier
 *       Return:  -1 en cas d'erreur, 0 sinon
 * =====================================================================================
 */
/* --------- copy_string --------- {{{ */
    int
copy_string (pstring_t *dest, const char *src)
{
    int len = strlen (src);

    if (!dest->str)
    {
        dest->str = (char *) calloc (sizeof (char), len + 1);
        if (!dest->str)
        {
            return -1;
        }

        dest->size = len + 1;
        dest->len = 0;
    }
    else if (dest->size <= len)
    {
        char *tmp_buf = (char *) realloc (dest->str, sizeof(char) * (len + 1));
        if (!tmp_buf) {
            return -1;
        }

        dest->str = tmp_buf;
        dest->size = len + 1;
        dest->len = 0;
    }

    strncpy (dest->str, src, len);
    dest->len = len;
    dest->str[len] = '\0';

    return 0;
}		/* -----  end of function copy_string  ----- */
/* }}} */
