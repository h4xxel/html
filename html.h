#ifndef __HTML_H_
#define __HTML_H_

#include "stack.h"

#ifndef HTML_PARSE_STATE_TYPE
#define HTML_PARSE_STATE_TYPE void
#endif

/*keep sorted, binary search*/
typedef enum HtmlTag HtmlTag;
enum HtmlTag {
	HTML_TAG_NONE,
	
	HTML_TAG_COMMENT,
	HTML_TAG_DOCTYPE,
	
	HTML_TAG_A,
	HTML_TAG_ABBR,
	HTML_TAG_ACRONYM,
	HTML_TAG_ADDRESS,
	HTML_TAG_APPLET,
	HTML_TAG_AREA,
	
	HTML_TAG_B,
	HTML_TAG_BASE,
	HTML_TAG_BASEFONT,
	HTML_TAG_BDO,
	HTML_TAG_BIG,
	HTML_TAG_BLOCKQUOTE,
	HTML_TAG_BODY,
	HTML_TAG_BR,
	HTML_TAG_BUTTON,
	
	HTML_TAG_CAPTION,
	HTML_TAG_CENTER,
	HTML_TAG_CITE,
	HTML_TAG_CODE,
	HTML_TAG_COL,
	HTML_TAG_COLGROUP,
	HTML_TAG_DD,
	HTML_TAG_DEL,
	
	HTML_TAG_DFN,
	HTML_TAG_DIR,
	HTML_TAG_DIV,
	HTML_TAG_DL,
	HTML_TAG_DT,
	
	HTML_TAG_EM,
	HTML_TAG_FIELDSET,
	HTML_TAG_FONT,
	HTML_TAG_FORM,
	HTML_TAG_FRAME,
	HTML_TAG_FRAMESET,
	
	HTML_TAG_H1,
	HTML_TAG_H2,
	HTML_TAG_H3,
	HTML_TAG_H4,
	HTML_TAG_H5,
	HTML_TAG_H6,
	HTML_TAG_HEAD,
	HTML_TAG_HR,
	HTML_TAG_HTML,
	
	HTML_TAG_I,
	HTML_TAG_IFRAME,
	HTML_TAG_IMG,
	HTML_TAG_INPUT,
	HTML_TAG_INS,
	
	HTML_TAG_KBD,
	
	HTML_TAG_LABEL,
	HTML_TAG_LEGEND,
	HTML_TAG_LI,
	HTML_TAG_LINK,
	
	HTML_TAG_MAP,
	HTML_TAG_MENU,
	HTML_TAG_META,
	
	HTML_TAG_NOFRAMES,
	HTML_TAG_NOSCRIPT,
	
	HTML_TAG_OBJECT,
	HTML_TAG_OL,
	HTML_TAG_OPTGROUP,
	HTML_TAG_OPTION,
	
	HTML_TAG_P,
	HTML_TAG_PARAM,
	HTML_TAG_PRE,
	
	HTML_TAG_Q,
	
	HTML_TAG_S,
	HTML_TAG_SAMP,
	HTML_TAG_SCRIPT,
	HTML_TAG_SELECT,
	HTML_TAG_SMALL,
	HTML_TAG_SPAN,
	HTML_TAG_STRIKE,
	HTML_TAG_STRONG,
	HTML_TAG_STYLE,
	HTML_TAG_SUB,
	HTML_TAG_SUP,
	
	HTML_TAG_TABLE,
	HTML_TAG_TBODY,
	HTML_TAG_TD,
	HTML_TAG_TEXTAREA,
	HTML_TAG_TFOOT,
	HTML_TAG_TH,
	HTML_TAG_THEAD,
	HTML_TAG_TITLE,
	HTML_TAG_TR,
	HTML_TAG_TT,
	
	HTML_TAG_U,
	HTML_TAG_UL,
	
	HTML_TAG_VAR,
	
	HTML_TAGS,
};

typedef enum HtmlAttribKey HtmlAttribKey;
enum HtmlAttribKey {
	HTML_ATTRIB_WIDTH,
	HTML_ATTRIB_HEIGHT,
	
	HTML_ATTRIB_KEYS,
};

typedef struct HtmlAttrib HtmlAttrib;
struct HtmlAttrib {
	HtmlAttribKey key;
	char *value;
	HtmlAttrib *next;
};

typedef struct HtmlElement HtmlElement;
struct HtmlElement {
	HtmlTag tag;
	char *text;
	HtmlAttrib *attrib;
	HtmlElement *child;
	HtmlElement *sibbling;
};

typedef struct HtmlDocument HtmlDocument;
struct HtmlDocument {
	HtmlElement *root_element;
};

typedef HTML_PARSE_STATE_TYPE HtmlParseState;


extern const char const *html_tag[HTML_TAGS];
extern const char const *html_attrib_key[HTML_ATTRIB_KEYS];

HtmlTag html_lookup_tag(const char *string);
HtmlParseState *html_parse_begin();
const char *html_parse_stream(HtmlParseState *state, const char *stream, const char *token, size_t len);
HtmlDocument *html_parse_end(HtmlParseState *state);
void *html_free_element(HtmlElement *element, int level);

// printing
void *html_print_tree(HtmlDocument *document);
void *html_print_tree_element(HtmlElement *element, int level);

void *html_free_document(HtmlDocument *document);

#endif
