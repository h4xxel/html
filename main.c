#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "html.h"

#define BUFFSIZE (1024*1024)

int main(int argc, char **argv) {
	HtmlDocument *doc;
	char buffer[BUFFSIZE + 1];
	FILE *f = fopen("/tmp/crap.html", "r");
	buffer[fread(buffer, 1, BUFFSIZE, f)] = 0;
	doc = html_parse_document(buffer);
	fclose(f);
	
	html_free_document(doc);
	
	return 0;
}
