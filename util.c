#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "util.h"

int stringcompare(const char *s1, const char *s2, size_t length) {
	int diff, i;
	for(i = 0; ; i++) {
		if(i >= length) {
			if(!s2[i])
				return 0;
			return -1;
		}
		diff = tolower(s1[i]) - tolower(s2[i]);
		if(diff)
			return diff;
		if(!s1[i]) {
			if(!s2[i])
				return 0;
			else
				return -1;
		}
		if(!s2[i]) {
			if(!s1[i])
				return 0;
			else
				return 1;
		}
	}
}

char *stringduplicate_length(const char *string, size_t len) {
	char *ret;
	size_t i, j;
	char space = 0;
	
	if(!(string && len))
		return NULL;
	if(!(ret = malloc(len + 1)))
		return NULL;
	for(i = 0, j = 0; j < len; i++) {
		if(isspace(string[i])) {
			if(space)
				continue;
			space = 1;
		} else
			space = 0;
		ret[j++] = string[i];
	}
	ret[len] = 0;
	return ret;
}

const char *stringtrim_l(const char *string) {
	if(!string)
		return NULL;
	while(*string && isspace(*string))
		string ++;
	
	return string;
}