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

string student_to_str(const student_t *s) {
	int day   = s->birthdate.tm_mday;
	int month = s->birthdate.tm_mon + 1;
	int year  = s->birthdate.tm_year + 1900;
	string s_student;
	// conversion de l'id :
	string id_string = "";
	size_t id_conversion = to_string(s->id).size();
	for (int i=0; i<9-(int)id_conversion; i++) {
		id_string += "0";
	}
	id_string += to_string(s->id);

	// conversion du birthday :
	string id_day;
	string id_month;
	size_t birthday_day_conversion = to_string(day).size();
	size_t birthday_month_conversion = to_string(month).size();
	for (int i=0; i<2-(int)birthday_day_conversion; i++) {
		id_day += "0";
	}
	id_day += to_string(day);
	for (int i=0; i<2-(int)birthday_month_conversion; i++) {
		id_month += "0";
	}
	id_month += to_string(month);
	s_student = id_string + ": " + s->fname + " " + s->lname + " in section " + s->section + ", born on the " +
	id_day + "/" + id_month + "/" + to_string(year) + "\n";
	return s_student;
}

bool student_equals(const student_t *s1, const student_t *s2) {
	return s1->id == s2->id && strcmp(s1->fname, s2->fname) == 0 &&
	strcmp(s1->lname, s2->lname) == 0 && strcmp(s1->section, s2->section) == 0 &&
	s1->birthdate.tm_mday == s2->birthdate.tm_mday && s1->birthdate.tm_mon == s2->birthdate.tm_mon &&
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
		if (date.tm_mday == 29 && date.tm_mon + 1 == 2 && 
		!((date.tm_year + 1900) % 4 == 0 && (date.tm_year + 1900) % 100 == 0 && (date.tm_year + 1900) % 400 == 0)) {
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
		if (date.tm_mday == 29 && date.tm_mon + 1 == 2 && 
		!((date.tm_year + 1900) % 4 == 0 && (date.tm_year + 1900) % 100 == 0 && (date.tm_year + 1900) % 400 == 0)) {
			return nullptr;
		}
		return [date](student_t& s) { s.birthdate = date; };
	} else {
		return nullptr;
	}
}
