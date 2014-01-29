#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "html.h"

/*keep sorted, binary search*/
const char const *html_tag[HTML_TAGS] = {
	[HTML_TAG_NONE] = "",
	[HTML_TAG_COMMENT] = "!--",
	[HTML_TAG_DOCTYPE] = "!doctype",
	
	[HTML_TAG_A] = "a",
	[HTML_TAG_ABBR] = "abbr",
	[HTML_TAG_ACRONYM] = "acronym",
	[HTML_TAG_ADDRESS] = "address",
	[HTML_TAG_APPLET] = "applet",
	[HTML_TAG_AREA] = "area",
	
	[HTML_TAG_B] = "b",
	[HTML_TAG_BASE] = "base",
	[HTML_TAG_BASEFONT] = "basefont",
	[HTML_TAG_BDO] = "bdo",
	[HTML_TAG_BIG] = "big",
	[HTML_TAG_BLOCKQUOTE] = "blockquote",
	[HTML_TAG_BODY] = "body",
	[HTML_TAG_BR] = "br",
	[HTML_TAG_BUTTON] = "button",
	
	[HTML_TAG_CAPTION] = "caption",
	[HTML_TAG_CENTER] = "center",
	[HTML_TAG_CITE] = "cite",
	[HTML_TAG_CODE] = "code",
	[HTML_TAG_COL] = "col",
	[HTML_TAG_COLGROUP] = "colgroup",
	[HTML_TAG_DD] = "dd",
	[HTML_TAG_DEL] = "del",
	
	[HTML_TAG_DFN] = "dfn",
	[HTML_TAG_DIR] = "dir",
	[HTML_TAG_DIV] = "div",
	[HTML_TAG_DL] = "dl",
	[HTML_TAG_DT] = "dt",
	
	[HTML_TAG_EM] = "em",
	[HTML_TAG_FIELDSET] = "fieldset",
	[HTML_TAG_FONT] = "font",
	[HTML_TAG_FORM] = "form",
	[HTML_TAG_FRAME] = "frame",
	[HTML_TAG_FRAMESET] = "frameset",
	
	[HTML_TAG_H1] = "h1",
	[HTML_TAG_H2] = "h2",
	[HTML_TAG_H3] = "h3",
	[HTML_TAG_H4] = "h4",
	[HTML_TAG_H5] = "h5",
	[HTML_TAG_H6] = "h6",
	[HTML_TAG_HEAD] = "head",
	[HTML_TAG_HR] = "hr",
	[HTML_TAG_HTML] = "html",
	
	[HTML_TAG_I] = "i",
	[HTML_TAG_IFRAME] = "iframe",
	[HTML_TAG_IMG] = "img",
	[HTML_TAG_INPUT] = "input",
	[HTML_TAG_INS] = "ins",
	
	[HTML_TAG_KBD] = "kbd",
	
	[HTML_TAG_LABEL] = "label",
	[HTML_TAG_LEGEND] = "legend",
	[HTML_TAG_LI] = "li",
	[HTML_TAG_LINK] = "link",
	
	[HTML_TAG_MAP] = "map",
	[HTML_TAG_MENU] = "menu",
	[HTML_TAG_META] = "meta",
	
	[HTML_TAG_NOFRAMES] = "noframes",
	[HTML_TAG_NOSCRIPT] = "noscript",
	
	[HTML_TAG_OBJECT] = "object",
	[HTML_TAG_OL] = "ol",
	[HTML_TAG_OPTGROUP] = "optgroup",
	[HTML_TAG_OPTION] = "option",
	
	[HTML_TAG_P] = "p",
	[HTML_TAG_PARAM] = "param",
	[HTML_TAG_PRE] = "pre",
	
	[HTML_TAG_Q] = "q",
	
	[HTML_TAG_S] = "s",
	[HTML_TAG_SAMP] = "samp",
	[HTML_TAG_SCRIPT] = "script",
	[HTML_TAG_SELECT] = "select",
	[HTML_TAG_SMALL] = "small",
	[HTML_TAG_SPAN] = "span",
	[HTML_TAG_STRIKE] = "strike",
	[HTML_TAG_STRONG] = "strong",
	[HTML_TAG_STYLE] = "style",
	[HTML_TAG_SUB] = "sub",
	[HTML_TAG_SUP] = "sup",
	
	[HTML_TAG_TABLE] = "table",
	[HTML_TAG_TBODY] = "tbody",
	[HTML_TAG_TD] = "td",
	[HTML_TAG_TEXTAREA] = "textarea",
	[HTML_TAG_TFOOT] = "tfoot",
	[HTML_TAG_TH] = "th",
	[HTML_TAG_THEAD] = "thead",
	[HTML_TAG_TITLE] = "title",
	[HTML_TAG_TR] = "tr",
	[HTML_TAG_TT] = "tt",
	
	[HTML_TAG_U] = "u",
	[HTML_TAG_UL] = "ul",
	
	[HTML_TAG_VAR] = "var",
};

struct Stack {
	void *item;
	struct Stack *next;
};

static void push(struct Stack **stack, void *item) {
	struct Stack *n;
	
	if(!stack)
		return;
	if(!(n = malloc(sizeof(struct Stack))))
		return;
	
	n->item = item;
	n->next = *stack;
	*stack = n;
}

static void *pop(struct Stack **stack) {
	struct Stack *p;
	void *item;
	
	if(!stack)
		return NULL;
	if(!(p = *stack))
		return NULL;
	
	*stack = p->next;
	item = p->item;
	free(p);
	
	return item;
}

static void *peek(struct Stack **stack) {
	if(!stack)
		return NULL;
	if(!*stack)
		return NULL;
	
	return (*stack)->item;
}

HtmlTag html_lookup_tag(const char *string) {
	int i, imin = 0, imax = HTML_TAGS, res;
	
	while(imax >= imin) {
		i = (imax - imin)/2 + imin;
		res = strcasecmp(string, html_tag[i]);
		if(res < 0)
			imax = i - 1;
		else if(res > 0)
			imin = i + 1;
		else
			return i;
	}
	
	return 0;
}

HtmlElement *html_new_element(HtmlTag tag, HtmlAttrib *attrib, HtmlElement *child, HtmlElement *sibbling) {
	HtmlElement *elem;
	if(!(elem = malloc(sizeof(HtmlElement))))
		return NULL;
	
	elem->tag = tag;
	elem->attrib = attrib;
	elem->child = child;
	elem->sibbling = sibbling;
	
	return elem;
}

HtmlDocument *html_parse_document(const char *string) {
	//int length;
	void *arne;
	char c;
	void *entity_p, *elem_p;
	HtmlTag tag;
	HtmlDocument *document;
	HtmlElement *elem = NULL, *elem_tmp;
	/*should we keep last sibling on the stack? might be sensible*/
	struct Stack *stack = NULL;
	
	enum State {
		STATE_CHILD,
		STATE_OPEN,
		STATE_ATTRIB,
		STATE_CLOSE,
		STATE_ENTITY,
		
		STATES,
	} state = STATE_CHILD;
	/*more states needed i think*/
	
	if(!string)
		return NULL;
	if(!(document = malloc(sizeof(HtmlDocument))))
		return NULL;
	if(!(document->root_element = html_new_element(HTML_TAG_NONE, NULL, NULL, NULL)))
		goto error;
	push(&stack, document->root_element);
	
	while((c = *string++)) {
		switch(state) {
			case STATE_CHILD:
				switch(c) {
					case '<':
						state = STATE_OPEN;
						tag = 0;
						elem_p = string;
						continue;
					case '&':
						state = STATE_ENTITY;
						entity_p = string;
						continue;
				}
			case STATE_OPEN:
				switch(c) {
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						if(!tag) {
							arne = strndup(elem_p, ((void *) (string - 1)) - elem_p);
							tag = html_lookup_tag(arne);
						}
						continue;
					case '<':
						/*push to stack, open another element.. perhaps*/
						continue;
					case '/':
						/*save poiter, figure out which element to close*/
						if(tag) {
							/*self closing*/
							
							continue;
						}
						state = STATE_CLOSE;
						elem_p = string;
						continue;
					case '>':
						/*fill in the blanks*/
						if(!(elem_tmp = html_new_element(HTML_TAG_NONE, NULL, NULL, NULL)))
							goto error;
						if(elem) {
							elem->sibbling = elem_tmp;
							elem = elem_tmp;
						} else {
							elem = peek(&stack);
							elem->child = elem_tmp;
							elem = elem_tmp;
						}
						arne = strndup(elem_p, ((void *) (string - 1)) - elem_p);
						if((tag = html_lookup_tag(arne)) < 0)
							tag = 0;
						free(arne);
						
						elem->tag = tag;
						
						state = STATE_CHILD;
						push(&stack, elem);
						elem = NULL;
						continue;
				}
			case STATE_CLOSE:
				/*find >*/
				/*run up stack to find the right element*/
				/**/
				switch(c) {
					case '>':
						/*fix*/
						arne = strndup(elem_p, ((void *) (string - 1)) - elem_p);
						if((tag = html_lookup_tag(arne)) < 0)
							tag = elem->tag;
						free(arne);
						
						do {
							/*check for null, broken pages*/
							elem_tmp = pop(&stack);
						} while(elem_tmp->tag != tag);
						
						elem = elem_tmp;
						
						state = STATE_CHILD;
						continue;
				}
			case STATE_ENTITY:
				if(c == ';') {
					/*parse entity at *p*/
					state = STATE_CHILD;
				}
			default:
				break;
		}
	}
	
	while(pop(&stack));
	
	return document;
	
	error:
	html_free_document(document);
	fprintf(stderr, "ERROR\n");
	return NULL;
}

void *html_free_element(HtmlElement *element, int level) {
	int i;
	if(!element)
		return NULL;
	while(element) {
		HtmlElement *sibbling = element->sibbling;
		for(i = 0; i < level; i++)
			printf("\t");
		printf("element %i\n", element->tag);
		html_free_element(element->child, level + 1);
		free(element);
		element = sibbling;
	}
	return NULL;
}

void *html_free_document(HtmlDocument *document) {
	if(!document)
		return NULL;
	html_free_element(document->root_element, 0);
	free(document);
	return NULL;
}
