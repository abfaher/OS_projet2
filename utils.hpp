#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Effectue un `read` en vérifiant la valeur de retour.
 * Si la valeur de retour est négative, la fonction affiche l'erreur et termine
 * le processus avec `exit(1)`.
 *
 * @param fd le file descriptor depuis lequel lire
 * @param buffer le buffer dans lequel mettre les données lues
 * @param nbytes le nombre de bytes maximum à lire
 * @return size_t le nombre de bytes lus
 */
size_t safe_read(int fd, void *buffer, size_t nbytes);

/**
 * @brief Effectue un `write` en vérifiant la valeur de retour.
 * Si la valeur de retour est négative, la fonction affiche l'erreur et termine
 * le processus avec `exit(1)`.
 *
 * @param fd le file descriptor dans lequel écrire
 * @param buffer le buffer dans lequel se trouvent les données
 * @param nbytes le nombre de bytes à écrire
 * @return size_t le nombre de bytes écrits
 */
size_t safe_write(int fd, const void *buffer, size_t nbytes);

#endif  // _UTILS_H