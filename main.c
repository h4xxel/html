#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "html.h"

int main(int argc, char **argv) {
	HtmlDocument *doc;
	char buffer[4097];
	FILE *f = fopen("test.html", "r");
	buffer[fread(buffer, 1, 4096, f)] = 0;
	doc = html_parse_document(buffer);
	fclose(f);
	
	html_free_document(doc);
	
	return 0;
}
