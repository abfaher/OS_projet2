#ifndef _QUERIES_HPP
#define _QUERIES_HPP

#include <cstdio>
#include <vector>
#include <string>

#include "db.hpp"
#include "student.hpp"

using namespace std;

#define ERRORCOLOR  "\033[31m"
#define SUCCESSCOLOR  "\033[32m"
#define COLOR_OFF   "\033[0m"

enum QUERY_STATUS {QUERY_SUCCESS, QUERY_FAILURE};

struct query_result_t {
    vector<student_t> students;
    char errorMessage[256];
    QUERY_STATUS status;

};

// execute_* //////////////////////////////////////////////////////////////////

void execute_select(query_result_t& result, database_t* const db, const char* const field, const char* const value);

void execute_update(query_result_t& result, database_t* const db, const char* const ffield, const char* const fvalue, 
const char* const efield, const char* const evalue);

void execute_insert(query_result_t& result, database_t* const db, const char* const fname, const char* const lname, 
const unsigned id, const char* const section, const tm birthdate);

void execute_delete(query_result_t& result, database_t* const db, const char* const field, const char* const value);

// parse_and_execute_* ////////////////////////////////////////////////////////

void parse_and_execute_select(query_result_t& result, const char* const query);

void parse_and_execute_update(query_result_t& result, database_t* db, const char* const query);

void parse_and_execute_insert(query_result_t& result, database_t* db, const char* const query);

void parse_and_execute_delete(query_result_t& result, database_t* db, const char* const query);

void parse_and_execute(query_result_t& result, database_t* db, const char* const query);

// query_fail_* ///////////////////////////////////////////////////////////////

/** Those methods write a descriptive error message on fout */

void query_fail_bad_query_type(query_result_t& result);

void query_fail_bad_format(query_result_t& result, const char* const query_type);

void query_fail_too_long(query_result_t& result);

void query_fail_bad_filter(query_result_t& result, const char* const field, const char* const filter);

void query_fail_bad_update(query_result_t& result, const char* const field, const char* const filter);

void query_fail_bad_birthdate(query_result_t& result, const char* const filter);

#endif
