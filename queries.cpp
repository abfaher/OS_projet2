#include <iostream>
#include <stdio.h>
#include "queries.hpp"

using namespace std;

//#include "io.hpp"

// execute_* ///////////////////////////////////////////////////////////////////

void execute_select(FILE* fout, database_t* const db, const char* const field,
                    const char* const value) {
  std::function<bool(const student_t&)> predicate = get_filter(field, value);
  int counter = 0;
  if (!predicate) {
    query_fail_bad_filter(fout, field, value);
    return;
  }
  for (const student_t& s : db->data) {
    if (predicate(s)) {
      fprintf(fout, "%d: %s %s in section %s, born on the %d/%d/%d\n", s.id, s.fname, s.lname,
    s.section, s.birthdate.tm_mday, s.birthdate.tm_mon + 1, s.birthdate.tm_year + 1900);
      counter++;
    }
  }
  fprintf(fout, "%d student(s) selected\n", counter);
}

void execute_update(FILE* fout, database_t* const db, const char* const ffield, const char* const fvalue, const char* const efield, const char* const evalue) {
  std::function<bool(const student_t&)> predicate = get_filter(ffield, fvalue);
  int counter = 0;
  if (!predicate) {
    query_fail_bad_filter(fout, ffield, fvalue);
    return;
  }
  std::function<void(student_t&)> updater = get_updater(efield, evalue);
  if (!updater) {
    query_fail_bad_update(fout, efield, evalue);
    return;
  }
  for (student_t& s : db->data) {
    if (predicate(s)) {
      counter++;
      updater(s);
    }
  }
  fprintf(fout, "%d student(s) selected\n", counter);
}

void execute_insert(FILE* fout, database_t* const db, const char* const fname,
                    const char* const lname, const unsigned id, const char* const section,
                    const tm birthdate) {
  db->data.emplace_back();
  student_t *s = &db->data.back();
  s->id = id;
  snprintf(s->fname, sizeof(s->fname), "%s", fname);
  snprintf(s->lname, sizeof(s->lname), "%s", lname);
  snprintf(s->section, sizeof(s->section), "%s", section);
  s->birthdate = birthdate;
  fprintf(fout, "%d: %s %s in section %s, born on the %d/%d/%d\n", s->id, s->fname, s->lname,
    s->section, s->birthdate.tm_mday, s->birthdate.tm_mon + 1, s->birthdate.tm_year + 1900);
}

void execute_delete(FILE* fout, database_t* const db, const char* const field,
                    const char* const value) {
  std::function<bool(const student_t&)> predicate = get_filter(field, value);
  if (!predicate) {
    query_fail_bad_filter(fout, field, value);
    return;
  }
  int counter = 0;
  size_t taille = db->data.size();
  for (size_t i=0; i < taille;  i++) {
    if (predicate(db->data[i])) {
      db->data[i] = db->data[db->data.size()-1];
      db->data.pop_back();
      i--;
      taille--;
      counter++;
    }
  }
  fprintf(fout, "%d student(s) deleted\n", counter);
  // auto new_end = remove_if(db->data.begin(), db->data.end(), predicate);
  // db->data.erase(new_end, db->data.end());
}

// parse_and_execute_* ////////////////////////////////////////////////////////

void parse_and_execute_select(FILE* fout, database_t* db, const char* const query) {
  char ffield[32], fvalue[64];  // filter data
  int counter;
  if (sscanf(query, "select %31[^=]=%63s%n", ffield, fvalue, &counter) != 2) {
    query_fail_bad_format(fout, "select");
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "select");
  } else {
    execute_select(fout, db, ffield, fvalue);
  }
}

void parse_and_execute_update(FILE* fout, database_t* db, const char* const query) {
  char ffield[32], fvalue[64];  // filter data
  char efield[32], evalue[64];  // edit data
  int counter;
  if (sscanf(query, "update %31[^=]=%63s set %31[^=]=%63s%n", ffield, fvalue, efield, evalue,
             &counter) != 4) {
    query_fail_bad_format(fout, "update");
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "update");
  } else {
    execute_update(fout, db, ffield, fvalue, efield, evalue);
  }
}

void parse_and_execute_insert(FILE* fout, database_t* db, const char* const query) {
  char      fname[64], lname[64], section[64], date[11];
  unsigned  id;
  tm        birthdate;
  int       counter;
  if (sscanf(query, "insert %63s %63s %u %63s %10s%n", fname, lname, &id, section, date, &counter) != 5 || strptime(date, "%d/%m/%Y", &birthdate) == NULL) {
    query_fail_bad_format(fout, "insert");
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "insert");
  } else {
    execute_insert(fout, db, fname, lname, id, section, birthdate);
  }
}

void parse_and_execute_delete(FILE* fout, database_t* db, const char* const query) {
  char ffield[32], fvalue[64]; // filter data
  int counter;
  if (sscanf(query, "delete %31[^=]=%63s%n", ffield, fvalue, &counter) != 2) {
    query_fail_bad_format(fout, "delete");
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "delete");
  } else {
    execute_delete(fout, db, ffield, fvalue);
  }
}

void parse_and_execute(FILE* fout, database_t* db, const char* const query) {
  if (strncmp("select", query, sizeof("select")-1) == 0) {
    parse_and_execute_select(fout, db, query);
  } else if (strncmp("update", query, sizeof("update")-1) == 0) {
    parse_and_execute_update(fout, db, query);
  } else if (strncmp("insert", query, sizeof("insert")-1) == 0) {
    parse_and_execute_insert(fout, db, query);
  } else if (strncmp("delete", query, sizeof("delete")-1) == 0) {
    parse_and_execute_delete(fout, db, query);
  } else {
    query_fail_bad_query_type(fout);
  }
}

// query_fail_* ///////////////////////////////////////////////////////////////

void query_fail_bad_query_type(FILE* fout) {
  cout << "fichier : " << fout << endl;
}

void query_fail_bad_format(FILE* fout, const char * const query_type) {
    cout << "ERROR : " << query_type << endl;
    cout << "fichier : " << fout << endl;
}

void query_fail_too_long(FILE* fout, const char * const query_type) {
  cout << "ERROR : " << query_type << endl;
  cout << "fichier : " << fout << endl;
}

void query_fail_bad_filter(FILE* fout, const char* const field, const char* const filter) {
  cout << "ERROR : " << field << filter << endl;
  cout << "fichier : " << fout << endl;
}

void query_fail_bad_update(FILE* fout, const char* const field, const char* const filter) {
  cout << "ERROR : " << field << filter << endl;
  cout << "fichier : " << fout << endl;
}

