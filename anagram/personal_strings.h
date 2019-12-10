/*
 * =====================================================================================
 *
 *       Filename:  personal_strings.h
 *
 *    Description:  Une bibliothèque pour gérer les chaînes de caractères de manière 
 *                      simple
 *
 *        Version:  1.0
 *        Created:  26/11/2019 10:01:49
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Guillerme Duvillié (mfreeze), guillerme@duvillie.eu
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef  __PERSONAL_STRINGS_H__
#define  __PERSONAL_STRINGS_H__

#include <stdio.h>

#ifndef INIT_STR_BUFFER_SIZE
#define	INIT_STR_BUFFER_SIZE 64			/* La taille initiale d'un buffer */
#endif

/*
 * =====================================================================================
 *    Structure:  pstring
 *  Description:  une chaîne de caractère 
 *      Members:  * char *str: le tampon mémoire contenant la chaîne de caractère
 *                * int len: la longueur de la chaîne de caractère
 *                * int size: la taille du tampon mémoire
 * =====================================================================================
 */
struct pstring {
    char *str;
    int len;
    int size;
};				/* ----------  end of struct pstring  ---------- */

typedef struct pstring pstring_t;

pstring_t empty_pstring ();

/*-----------------------------------------------------------------------------
 *  Gestion Mémoire
 *-----------------------------------------------------------------------------*/
/* {{{ -------- Gestion Mémoire -------- */
int init_pstring (pstring_t *pstr, int init_size);
void free_pstring (pstring_t *str);
/* }}} */

int secure_read_line (pstring_t *pstr, FILE *stream);
int copy_pstring (pstring_t *dest, const pstring_t *src);
int copy_string (pstring_t *dest, const char *src);
int ncopy_string (pstring_t *dest, const char *src, int len);

#endif   /* ----- #ifndef __PERSONAL_STRINGS_H__  ----- */
