#ifndef _QUERIES_HPP
#define _QUERIES_HPP

#include <cstdio>

#include "db.hpp"

// execute_* //////////////////////////////////////////////////////////////////

void execute_select(FILE* fout, database_t* const db, const char* const field,
                    const char* const value);

void execute_update(FILE* fout, database_t* const db, const char* const ffield,
                    const char* const fvalue, const char* const efield, const char* const evalue);

void execute_insert(FILE* fout, database_t* const db, const char* const fname,
                    const char* const lname, const unsigned id, const char* const section,
                    const tm birthdate);

void execute_delete(FILE* fout, database_t* const db, const char* const field,
                    const char* const value);

// void execute_dump(FILE* fout, database_t* const db);

// parse_and_execute_* ////////////////////////////////////////////////////////

void parse_and_execute_select(FILE* fout, const char* const query);

void parse_and_execute_update(FILE* fout, database_t* db, const char* const query);

void parse_and_execute_insert(FILE* fout, database_t* db, const char* const query);

void parse_and_execute_delete(FILE* fout, database_t* db, const char* const query);

void parse_and_execute(FILE* fout, database_t* db, const char* const query);

// query_fail_* ///////////////////////////////////////////////////////////////

/** Those methods write a descriptive error message on fout */

void query_fail_bad_query_type(FILE* fout);

void query_fail_bad_format(FILE* fout, const char* const query_type);

void query_fail_too_long(FILE* fout, const char* const query_type);

void query_fail_bad_filter(FILE* fout, const char* const field, const char* const filter);

void query_fail_bad_update(FILE* fout, const char* const field, const char* const filter);

#endif
