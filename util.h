/*
 * html - a simple html parser lacking a better name 
 * The contents of this file is licensed under the MIT License,
 * see the file COPYING or http://opensource.org/licenses/MIT
 */

#ifndef __HTML_UTIL__
#define __HTML_UTIL__

int stringcompare(const char *s1, const char *s2, size_t length);
char *stringduplicate_length(const char *string, size_t len);
const char *stringtrim_l(const char *string);

#endif
