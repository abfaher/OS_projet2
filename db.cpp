#include "db.hpp"

#include <err.h>       // err
#include <fcntl.h>     // open
#include <sys/stat.h>  // stat
#include <unistd.h>    // read

#include <algorithm>  // std::min
#include <cassert>    // assert
#include <climits>    // SSIZE_MAX
#include <cstdio>     // printf
#include <utility>    // std::move

#include "errorcodes.hpp"

void db_load(database_t *db, const char *path) {
  db->path = path;

  // Ouvrir le fichier et déterminer sa taille
  struct stat info;
  int fd_db = open(path, O_RDONLY);

  if (fd_db < 0) {
    warn("Unable to open %s (loading DB)", path);
    warnx("Starting with an empty DB.");
    db->data.reserve(100);
    return;
  }
  if (fstat(fd_db, &info) != 0) {
    err(FILE_ERROR, "Unable to stat %s (loading DB)", path);
  }
  if (info.st_size < 0) {
    err(FILE_ERROR, "Unable to stat %s (loading DB): get a negative size", path);
  }
  size_t size = static_cast<size_t>(info.st_size) / sizeof(student_t);
  if (info.st_size != static_cast<ssize_t>(sizeof(student_t) * size)) {
    err(FILE_ERROR, "Corrupted DB file");
  }

  // Initialiser la BDD (en RAM)
  assert(db->data.empty());
  db->data.reserve(size + size/2);

  // Charger la BDD (en RAM)
  for (size_t i = 0; i < size; ++i) {
    student_t s;
    ssize_t   r = read(fd_db, &s, sizeof(s));
    if (r < 0) {
      err(FILE_ERROR, "Unable to read the DB file");
    }
    if (static_cast<unsigned>(r) < sizeof(s)) {
      err(FILE_ERROR, "Corrupted DB file");
    }
    db->data.push_back(std::move(s)); // move Convert a value to an rvalue.
  }

  // Fermer le fichier
  if (close(fd_db) < 0) {
    err(FILE_ERROR, "Error while closing %s (after DB load)", path);
  }

  printf("%lu students found in the db.\n", size);

  // Le code ci-dessus n'est pas performant
  // à cause du trop grand nombre d'appel à read
  // et de la copie de chaque étudiant dans data.
  // L'utilisation de fopen, fread, etc. aurait
  // réglé le problème de lecture (ces fonctions
  // utilisent un buffer pour limiter le nombre d'appels
  // systèmes (read, write)), mais nous voulons illuster
  // ici le fonctionnement de ces appels systèmes.
  // (Dans un vrai code il faudrait utiliser fopen, fread...)
  //
  // Vous pouvez utiliser cette fonction dans votre projet.
  // Le défaut mentionner ne sera pas considéré.
}

void db_add(database_t *db, student_t s) { db->data.push_back(s); }

size_t db_delete(database_t *db, student_t *s) {
  size_t deleted = 0;
  for (std::vector<student_t>::iterator it = db->data.begin();
       it != db->data.end();) {
    if (student_equals(&*it, s)) {
      it = db->data.erase(it);
      ++deleted;
    } else {
      ++it;
    }
  }
  return deleted;
}

void db_save(database_t *db) {
  int fd_db = open(db->path, O_WRONLY | O_CREAT | O_TRUNC, 0640);
  if (fd_db < 0) {
    err(FILE_ERROR, "Unable to open %s (saving DB)", db->path);
  }
  const char  *raw_data    = reinterpret_cast<char *>(db->data.data());
  const size_t total_bytes = db->data.size() * sizeof(student_t);
  size_t       left_bytes  = total_bytes;
  while (left_bytes > 0) {
    // write() can not write more than SSIZE_MAX bytes
    size_t  to_write_bytes = std::min(static_cast<size_t>(SSIZE_MAX), left_bytes);
    ssize_t written =
        write(fd_db, &raw_data[total_bytes - left_bytes], to_write_bytes);
    if (written < 0) {
      warn("Unable to write DB to %s", db->path);
      warn("And %s was already cleared for writing!", db->path);
      return;
    }
    left_bytes -= static_cast<size_t>(written);
  }
}
