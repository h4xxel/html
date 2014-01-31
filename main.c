#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "html.h"

#define BUFFSIZE 1024

int main(int argc, char **argv) {
	//TODO: dynamic buffer handling to never overflow
	FILE *f;
	HtmlDocument *doc;
	HtmlParseState *parse_state;
	char buffer[BUFFSIZE + 1];
	const char *token = buffer;
	
	if(argc != 2) {
		fprintf(stderr, "usage: html <file.html>\n");
		return 1;
	}
	
	buffer[BUFFSIZE] = 0;
	size_t len = 0;
	size_t buffsize = BUFFSIZE;
	
	parse_state = html_parse_begin();
	if(!(f = fopen(argv[1], "r"))) {
		fprintf(stderr, "error: cannot open file %s\n", argv[1]);
		return 1;
	}
	
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
