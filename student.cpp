#include "student.hpp"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <ctime>

// private functions //////////////////////////////////////////////////////////

/** Return true if str matches the regexp /[+-]?0+/:
 * an optional sign followed uniquely by one or more 0
 */
bool is_str_zero(const char* const str) {
  if (*str == '\0') { // empty string
    return false;
  }
  // sign without any 0
  if ((*str == '+' || *str == '-') && *(str+1) == '\0') {
    return false;
  }
  // general case
  for (size_t i = 0; *(str + i) != '\0'; ++i) {
    if (*(str+i) != '0') {
      if (i != 0 || (*str != '+' && *str != '-')) {
        return false;
      }
    }
  }
  return true;
}

// main functions ////////////////////////////////////////////////////////////

void student_to_str(char *buffer, const student_t *s, size_t buffer_size) {
  int day   = s->birthdate.tm_mday;
  int month = s->birthdate.tm_mon;
  int year  = s->birthdate.tm_year;
  snprintf(buffer, buffer_size, "%.9u: %s %s in section %s, born on the %.2d/%.2d/%.2d",
          s->id, s->fname, s->lname, s->section, day, month + 1, year + 1900);
}

bool student_equals(const student_t *s1, const student_t *s2) {
  return s1->id == s2->id && strcmp(s1->fname, s2->fname) == 0 &&
         strcmp(s1->lname, s2->lname) == 0 &&
         strcmp(s1->section, s2->section) == 0 &&
         s1->birthdate.tm_mday == s2->birthdate.tm_mday &&
         s1->birthdate.tm_mon == s2->birthdate.tm_mon &&
         s1->birthdate.tm_year == s2->birthdate.tm_year;
}

// Helper functions ///////////////////////////////////////////////////////////

std::function<bool(const student_t&)> get_filter(const char* const field, const char* const value) {
  if (strcmp(field, "id") == 0) {
    unsigned ival = static_cast<unsigned>(strtoul(value, nullptr, 10));
    if (ival == 0 && !is_str_zero(value)) {
      return nullptr;
    }
    return [ival](const student_t& s) { return s.id == ival; };
  } else if (strcmp(field, "fname") == 0) {
    return [value](const student_t& s) { return strcmp(s.fname, value) == 0; };
  } else if (strcmp(field, "lname") == 0) {
    return [value](const student_t& s) { return strcmp(s.lname, value) == 0; };
  } else if (strcmp(field, "section") == 0) {
    return [value](const student_t& s) { return strcmp(s.section, value) == 0; };
  } else if (strcmp(field, "birthdate") == 0) {
    tm date;
    if (!strptime(value, "%d/%m/%Y", &date)) {
      return nullptr;
    }
    return [date](const student_t& s) {
      return s.birthdate.tm_year == date.tm_year && s.birthdate.tm_mon == date.tm_mon &&
             s.birthdate.tm_mday == date.tm_mday;
    };
  } else {
    return nullptr;
  }
}

std::function<void(student_t&)> get_updater(const char* const field, const char* const value) {
  if (strcmp(field, "id") == 0) {
    unsigned ival = static_cast<unsigned>(strtoul(value, nullptr, 10));
    if (ival == 0 && !is_str_zero(value)) {
      return nullptr;
    }
    return [ival](student_t& s) { s.id = ival; };
  } else if (strcmp(field, "fname") == 0) {
    return [value](student_t& s) { snprintf(s.fname, sizeof(s.fname), "%s", value); };
  } else if (strcmp(field, "lname") == 0) {
    return [value](student_t& s) { snprintf(s.lname, sizeof(s.lname), "%s", value); };
  } else if (strcmp(field, "section") == 0) {
    return [value](student_t& s) { snprintf(s.section, sizeof(s.section), "%s", value); };
  } else if (strcmp(field, "birthdate") == 0) {
    tm date;
    if (!strptime(value, "%d/%m/%Y", &date)) {
      return nullptr;
    }
    return [date](student_t& s) { s.birthdate = date; };
  } else {
    return nullptr;
  }
}
