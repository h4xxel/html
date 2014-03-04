#ifndef __ATTRIB_H_
#define __ATTRIB_H_

typedef enum HtmlAttribKey HtmlAttribKey;
enum HtmlAttribKey {
	HTML_ATTRIB_NONE,
	
	HTML_ATTRIB_WIDTH,
	HTML_ATTRIB_HEIGHT,
	
	HTML_ATTRIB_HREF,
	
	HTML_ATTRIB_KEYS,
};

typedef struct HtmlAttrib HtmlAttrib;
struct HtmlAttrib {
	HtmlAttribKey key;
	char *value;
	HtmlAttrib *next;
};

HtmlAttrib *attrib_append(HtmlAttrib **attrib, HtmlAttrib *item);

#endif
