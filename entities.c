#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "entities.h"

/*keep sorted, binary search*/
const char const *html_entities[HTML_ENTITY_LAST] = {
	[HTML_ENTITY_ae] = "auml",
	[HTML_ENTITY_AE] = "Auml",
	[HTML_ENTITY_AMP] = "amp",
	
	[HTML_ENTITY_COPY] = "copy",
	
	[HTML_ENTITY_EURO] = "euro",
	
	[HTML_ENTITY_GT] = "gt",
	
	[HTML_ENTITY_NBSP] = "nbsp",
	
	[HTML_ENTITY_oe] = "ouml",
	[HTML_ENTITY_OE] = "Ouml",
	
	[HTML_ENTITY_QUOT] = "quot",
	
	[HTML_ENTITY_SZ] = "szlig",
	
	[HTML_ENTITY_ue] = "uuml",
	[HTML_ENTITY_UE] = "Uuml",
};
const char html_entities_length[HTML_ENTITY_LAST] = {
	[HTML_ENTITY_ae] = 4,
	[HTML_ENTITY_AE] = 4,
	[HTML_ENTITY_AMP] = 3,
	
	[HTML_ENTITY_COPY] = 4,
	
	[HTML_ENTITY_EURO] = 4,
	
	[HTML_ENTITY_GT] = 2,
	
	[HTML_ENTITY_NBSP] = 4,
	
	[HTML_ENTITY_oe] = 4,
	[HTML_ENTITY_OE] = 4,
	
	[HTML_ENTITY_QUOT] = 4,
	
	[HTML_ENTITY_SZ] = 5,
	
	[HTML_ENTITY_ue] = 4,
	[HTML_ENTITY_UE] = 4,
};

/*keep sorted, binary search*/
const char const *html_entities_locale[HTML_ENTITY_LAST] = {
	[HTML_ENTITY_ae] = "ä",
	[HTML_ENTITY_AE] = "Ä",
	[HTML_ENTITY_AMP] = "&",
	
	[HTML_ENTITY_COPY] = "©",
	
	[HTML_ENTITY_EURO] = "€",
	
	[HTML_ENTITY_GT] = ">",
	
	[HTML_ENTITY_NBSP] = " ",
	
	[HTML_ENTITY_oe] = "ö",
	[HTML_ENTITY_OE] = "Ö",
	
	[HTML_ENTITY_QUOT] = "\"",
	
	[HTML_ENTITY_SZ] = "ß",
	
	[HTML_ENTITY_ue] = "ü",
	[HTML_ENTITY_UE] = "Ü",
};
const char html_entities_locale_length[HTML_ENTITY_LAST] = {
	[HTML_ENTITY_ae] = 2,
	[HTML_ENTITY_AE] = 2,
	[HTML_ENTITY_AMP] = 1,
	
	[HTML_ENTITY_COPY] = 2,
	
	[HTML_ENTITY_EURO] = 2,
	
	[HTML_ENTITY_GT] = 1,
	
	[HTML_ENTITY_NBSP] = 1,
	
	[HTML_ENTITY_oe] = 2,
	[HTML_ENTITY_OE] = 2,
	
	[HTML_ENTITY_QUOT] = 1,
	
	[HTML_ENTITY_SZ] = 2,
	
	[HTML_ENTITY_ue] = 2,
	[HTML_ENTITY_UE] = 2,
};

int html_entity_lookup(const char *string) {
	int i, imin = 0, imax = HTML_ENTITY_LAST, res;
	
	while(imax >= imin) {
		i = (imax - imin)/2 + imin;
		if (i==HTML_ENTITY_LAST)
			break;
		res = stringcompare(string, html_entities[i], strlen(html_entities[i]));
		if(res < 0)
			imax = i - 1;
		else if(res > 0)
			imin = i + 1;
		else
			return i;
	}
	return 0;
}

void *html_replace_entities(char **text, size_t length)
{
	size_t i;
	int entity_tag;
	int entity_tag_length;
	int entity_tag_locale_length;
	
	for(i = 0; i < length; i++) {
		if(*(*text+i) == '&') {
			entity_tag = html_entity_lookup(*text+i+1);
			if (entity_tag == HTML_ENTITY_NONE) {
				continue;
			}
			entity_tag_locale_length = html_entities_locale_length[entity_tag];
			memcpy(*text+i, html_entities_locale[entity_tag], entity_tag_locale_length);
			
			i += entity_tag_locale_length;
			entity_tag_length = html_entities_length[entity_tag];
			if (entity_tag_locale_length == 1) {
				memcpy(*text+i, *text+i+entity_tag_length+1, length - i - entity_tag_length);
				length -= entity_tag_length + 1;
			} else if (entity_tag_locale_length == 2) {
				memcpy(*text+i, *text+i+entity_tag_length, length - i - entity_tag_length + 1);
				length -= entity_tag_length;
			}
		}
	}
	return 0;
}