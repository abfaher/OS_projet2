#ifndef _STUDENT_HPP
#define _STUDENT_HPP

#include <cstring>
#include <ctime>  // std::tm
#include <functional>
#include <string>
#include <iostream>

using namespace std;

/**
 * Student structure type.
 **/
struct student_t {
  unsigned id;          /** Unique ID **/
  char     fname[64];   /** Firstname **/
  char     lname[64];   /** Lastname **/
  char     section[64]; /** Section **/
  std::tm  birthdate;   /** Birth date **/
};

/**
 * return a convertion of student to a human-readlable string.
 **/
string student_to_str(const student_t* s);

/**
 * Return whether two students are equal or not.
 **/
bool student_equals(const student_t* s1, const student_t* s2);

// Helper functions //////////////////////////////////////////////////////////

/** Return a pointer to a unary function that return true if the given student
 * match the filter field=value.
 */
std::function<bool(const student_t&)> get_filter(const char* const field, const char* const value);

/** Return a pointer to a unary function that apply field=value to the given
 * student.
 */
std::function<void(student_t&)> get_updater(const char* const field, const char* const value);

#endif  // _STUDENT_HPP
