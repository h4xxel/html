#include <stdlib.h>
#include <string.h>
#include "attrib.h"

static HtmlAttrib *attrib_get_last(HtmlAttrib *item) {
	if (!item) {
		return NULL;
	}
	while(item->next) {
		item = item->next;
	}
	return item;
}

HtmlAttrib *attrib_append(HtmlAttrib **attrib, HtmlAttrib *item) {
	HtmlAttrib *attrib_last;
	
	if (!*attrib) {
		*attrib = malloc(sizeof(HtmlAttrib));
		memcpy(*attrib, item, sizeof(HtmlAttrib));
	} else {
		attrib_last = attrib_get_last(*attrib);
		attrib_last->next = item;
	}
	
	return item;
}