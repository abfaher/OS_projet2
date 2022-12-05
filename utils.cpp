#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.hpp"

size_t safe_read(int fd, void* buffer, size_t nbytes) {
	/* Lit dans le file descriptor et renvoie une erreur si la lecture s'est mal déroulée */
	ssize_t nbytes_read = read(fd, buffer, nbytes);
	if (nbytes_read < 0) {
		perror("read error: ");
		exit(1);
	}
	return (size_t)nbytes_read;
}

size_t safe_write(int fd, const void* buffer, size_t nbytes) {
	/* Ecrit dans le file descriptor et renvoie une erreur si l'écriture s'est mal déroulée */
	ssize_t bytes_written = write(fd, buffer, nbytes);
	if (bytes_written < 0) {
		perror("write: ");
		exit(1);
	}
	return (size_t)bytes_written;
}