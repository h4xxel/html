#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "html.h"

#define BUFFSIZE 1024

int main(int argc, char **argv) {
	HtmlDocument *doc;
	HtmlParseState *parse_state;
	char buffer[BUFFSIZE + 1];
	const char *token = buffer;
	buffer[BUFFSIZE] = 0;
	size_t len = 0;
	size_t buffsize = BUFFSIZE;
	
	parse_state = html_parse_begin();
	FILE *f = fopen("/tmp/crap.html", "r");
	
	while(!feof(f)) {
		len = fread(buffer + (BUFFSIZE - buffsize), 1, buffsize, f);
		token = html_parse_stream(parse_state, buffer + (BUFFSIZE - buffsize), buffer, len);
		buffsize = (token - buffer);
		memmove(buffer, token, BUFFSIZE - buffsize);
	}
	
	doc = html_parse_end(parse_state);
	fclose(f);
	
	html_free_document(doc);
	
	return 0;
}
