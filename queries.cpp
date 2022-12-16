#include <iostream>
#include <stdio.h>	
#include "queries.hpp"

using namespace std;

//#include "io.hpp"

// execute_* ///////////////////////////////////////////////////////////////////

void execute_select(query_result_t& result, database_t* const db, const char* const field, const char* const value) {
	std::function<bool(const student_t&)> predicate = get_filter(field, value);
	if (!predicate) { query_fail_bad_filter(result, field, value); } 
	else {
		pthread_mutex_lock(&db->exclusiveAccess);
		pthread_mutex_lock(&db->readAccess);
		if (db->readers_c == 0) { pthread_mutex_lock(&db->writeAccess); }
		db->readers_c++;
		pthread_mutex_unlock(&db->exclusiveAccess);
		pthread_mutex_unlock(&db->readAccess);
		for (const student_t& s : db->data) {
			if (predicate(s)) { result.students.push_back(s); }
		}
		result.status = QUERY_SUCCESS;
		pthread_mutex_lock(&db->exclusiveAccess);
		db->readers_c--;
		if (db->readers_c == 0) { pthread_mutex_unlock(&db->writeAccess); }
		pthread_mutex_unlock(&db->exclusiveAccess);
	}
}

void execute_update(query_result_t& result, database_t* const db, const char* const ffield, const char* const fvalue, 
const char* const efield, const char* const evalue) {
	std::function<bool(const student_t&)> predicate = get_filter(ffield, fvalue);
	if (!predicate) { query_fail_bad_filter(result, ffield, fvalue); } 
	else {
		std::function<void(student_t&)> updater = get_updater(efield, evalue);
		if (!updater) { query_fail_bad_update(result, efield, evalue); } 
		else if (strcmp(efield, "id") == 0) {
			execute_select(result, db, efield, evalue);
			if (result.students.size() > 0) {
				result.status = QUERY_FAILURE;
				strcpy(result.errorMessage, ERRORCOLOR "ERROR: trying to update to an id that is already used\n" COLOR_OFF);
			}
		} else {
			pthread_mutex_lock(&db->exclusiveAccess);
			pthread_mutex_lock(&db->writeAccess);
			pthread_mutex_unlock(&db->exclusiveAccess);
			for (student_t& s : db->data) {
				if (predicate(s)) { updater(s); result.students.push_back(s); }
			}
			result.status = QUERY_SUCCESS;
			pthread_mutex_unlock(&db->writeAccess);
		}
	}
}

void execute_insert(query_result_t& result, database_t* const db, const char* const fname, const char* const lname, const unsigned id, 
const char* const section, const tm birthdate) {
	execute_select(result, db, "id", to_string(id).c_str());
	if (result.students.size() > 0) {
		result.status = QUERY_FAILURE;
		strcpy(result.errorMessage, ERRORCOLOR "ERROR: trying to insert a student with an id that is already used\n" COLOR_OFF);
	} else {
		pthread_mutex_lock(&db->exclusiveAccess);
		pthread_mutex_lock(&db->writeAccess);
		pthread_mutex_unlock(&db->exclusiveAccess);
		result.students.clear();
		db->data.emplace_back();
		student_t *s = &db->data.back();
		result.students.clear();
		s->id = id;
		snprintf(s->fname, sizeof(s->fname), "%s", fname);
		snprintf(s->lname, sizeof(s->lname), "%s", lname);
		snprintf(s->section, sizeof(s->section), "%s", section);
		s->birthdate = birthdate;
		result.students.push_back(*s);
		result.status = QUERY_SUCCESS;
		pthread_mutex_unlock(&db->writeAccess);
	}
}

void execute_delete(query_result_t& result, database_t* const db, const char* const field, const char* const value) {
	std::function<bool(const student_t&)> predicate = get_filter(field, value);
	if (!predicate) { query_fail_bad_filter(result, field, value); } 
	else {
		pthread_mutex_lock(&db->exclusiveAccess);
		pthread_mutex_lock(&db->writeAccess);
		pthread_mutex_unlock(&db->exclusiveAccess);
		size_t taille = db->data.size();
		for (size_t i=0; i < taille; i++) {
			if (predicate(db->data[i])) {
				result.students.push_back(db->data[i]);
				db->data[i] = db->data[db->data.size()-1];
				db->data.pop_back();
				i--;
				taille--;
			}
		}
		result.status = QUERY_SUCCESS;
		pthread_mutex_unlock(&db->writeAccess);
	}
}

// parse_and_execute_* ////////////////////////////////////////////////////////

void parse_and_execute_select(query_result_t& result, database_t* db, const char* const query) {
	char ffield[32], fvalue[64];  // filter data
	int counter;
	if (sscanf(query, "select %31[^=]=%63s%n", ffield, fvalue, &counter) != 2) { query_fail_bad_format(result, "select"); } 
	else if (static_cast<unsigned>(counter) < strlen(query)) { query_fail_too_long(result); } 
	else { execute_select(result, db, ffield, fvalue); }
}

void parse_and_execute_update(query_result_t& result, database_t* db, const char* const query) {
	char ffield[32], fvalue[64];  // filter data
	char efield[32], evalue[64];  // edit data
	int counter;
	if (sscanf(query, "update %31[^=]=%63s set %31[^=]=%63s%n", ffield, fvalue, efield, evalue, &counter) != 4) {
		query_fail_bad_format(result, "update");
	} 
	else if (static_cast<unsigned>(counter) < strlen(query)) { query_fail_too_long(result); } 
	else { execute_update(result, db, ffield, fvalue, efield, evalue); }
}

void parse_and_execute_insert(query_result_t& result, database_t* db, const char* const query) {
	char      fname[64], lname[64], section[64], date[11];
	unsigned  id;
	tm        birthdate;
	int       counter;
	if (sscanf(query, "insert %63s %63s %u %63s %10s%n", fname, lname, &id, section, date, &counter) != 5 || 
	strptime(date, "%d/%m/%Y", &birthdate) == NULL) { query_fail_bad_format(result, "insert"); } 
	else if (birthdate.tm_mday == 29 && birthdate.tm_mon + 1 == 2 && 
	!((birthdate.tm_year + 1900) % 4 == 0 && (birthdate.tm_year + 1900) % 100 == 0 && (birthdate.tm_year + 1900) % 400 == 0)) {
		query_fail_bad_birthdate(result, date);
	} 
	else if (static_cast<unsigned>(counter) < strlen(query)) { query_fail_too_long(result); } 
	else { execute_insert(result, db, fname, lname, id, section, birthdate); }
}

void parse_and_execute_delete(query_result_t& result, database_t* db, const char* const query) {
	char ffield[32], fvalue[64]; // filter data
	int counter;
	if (sscanf(query, "delete %31[^=]=%63s%n", ffield, fvalue, &counter) != 2) { query_fail_bad_format(result, "delete"); } 
	else if (static_cast<unsigned>(counter) < strlen(query)) { query_fail_too_long(result); } 
	else { execute_delete(result, db, ffield, fvalue); }
}

void parse_and_execute(query_result_t& result, database_t* db, const char* const query) {
	if (strncmp("select", query, sizeof("select")-1) == 0) { parse_and_execute_select(result, db, query); } 
	else if (strncmp("update", query, sizeof("update")-1) == 0) { parse_and_execute_update(result, db, query); } 
	else if (strncmp("insert", query, sizeof("insert")-1) == 0) { parse_and_execute_insert(result, db, query); } 
	else if (strncmp("delete", query, sizeof("delete")-1) == 0) { parse_and_execute_delete(result, db, query); } 
	else { query_fail_bad_query_type(result); }
}

// query_fail_* ///////////////////////////////////////////////////////////////

void query_fail_bad_query_type(query_result_t& result) {
	result.status = QUERY_FAILURE;
	strcpy(result.errorMessage, ERRORCOLOR "ERROR: unknown query type\n" COLOR_OFF);
}

void query_fail_bad_format(query_result_t& result, const char * const query_type) {
	result.status = QUERY_FAILURE;
	sprintf(result.errorMessage, ERRORCOLOR "ERROR: syntax error in %s\n" COLOR_OFF, query_type);
}

void query_fail_too_long(query_result_t& result) {
	result.status = QUERY_FAILURE;
	strcpy(result.errorMessage, ERRORCOLOR "ERROR: the query is too long\n" COLOR_OFF);
}

void query_fail_bad_filter(query_result_t& result, const char* const field, const char* const filter) {
	result.status = QUERY_FAILURE;
	sprintf(result.errorMessage, ERRORCOLOR "ERROR: '%s=%s' is not a valid filter\n" COLOR_OFF, field, filter);
}

void query_fail_bad_update(query_result_t& result, const char* const field, const char* const filter) {
	result.status = QUERY_FAILURE;
	sprintf(result.errorMessage, ERRORCOLOR "ERROR: you cannot apply '%s=%s'\n" COLOR_OFF, field, filter);
}

void query_fail_bad_birthdate(query_result_t& result, const char* const filter) {
	result.status = QUERY_FAILURE;
	sprintf(result.errorMessage, ERRORCOLOR "ERROR: %s is not a valid birthdate\n" COLOR_OFF, filter);
}
