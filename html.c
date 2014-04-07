/*
 * html - a simple html parser lacking a better name 
 * The contents of this file is licensed under the MIT License,
 * see the file COPYING or http://opensource.org/licenses/MIT
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Explicitly expose internal structures */
#define HTML_PARSE_STATE_TYPE struct HtmlParseState

#include "stack.h"
#include "attrib.h"
#include "html.h"
#include "util.h"

#define CASE_SPACE case ' ': case '\r': case '\n': case '\t'

enum State {
	STATE_CHILD,
	STATE_OPEN,
	STATE_DECLARATION,
	STATE_BEGIN,
	STATE_END,
	STATE_ATTRIB,
	STATE_ATTRIB_KEY,
	STATE_ATTRIB_VALUE,
	STATE_ATTRIB_QUOTEVALUE,
	STATE_CLOSE,
	STATE_SELFCLOSE,
	STATE_END_CLOSE,
	STATE_ENTITY,
	
	/*This is silly*/
	STATE_COMMENT_BEGIN,
	STATE_COMMENT,
	STATE_COMMENT_END1,
	STATE_COMMENT_END2,
	
	STATES,
};

struct HtmlParseState {
	HtmlDocument *document;
	Stack *stack;
	HtmlElement *elem;
	HtmlTag tag;
	char *tag_name;
	
	HtmlAttrib *attrib;
	HtmlAttribKey attrib_key;
	char *attrib_key_name;
	
	enum State state;
	
	/* Used for stripping out spaces */
	size_t stringlen;
	char space;
};

static int findtag(void *elem, void *tag) {
	if(((HtmlElement *) elem)->tag == *((int *) tag))
		return 1;
	return 0;
}

int html_tag_is_script(HtmlTag tag) {
	switch(tag) {
		case HTML_TAG_SCRIPT:
		case HTML_TAG_STYLE:
			return 1;
		default:
			return 0;
	}
}

int html_tag_is_selfclose(HtmlTag tag) {
	switch(tag) {
		case HTML_TAG_BASE:
		case HTML_TAG_BASEFONT:
		case HTML_TAG_FRAME:
		case HTML_TAG_LINK:
		case HTML_TAG_META:
		case HTML_TAG_AREA:
		case HTML_TAG_BR:
		case HTML_TAG_COL:
		case HTML_TAG_HR:
		case HTML_TAG_IMG:
		case HTML_TAG_INPUT:
		case HTML_TAG_PARAM:
			return 1;
		default:
			return 0;
	}
}

HtmlAttribKey html_lookup_length_attrib_key(const char *string, size_t length) {
	int i, imin = 0, imax = HTML_ATTRIB_KEYS, res;
	
	while(imax >= imin) {
		i = (imax - imin)/2 + imin;
		if (i==HTML_ATTRIB_KEYS)
			break;
		res = stringcompare(string, html_attrib[i], length);
		if(res < 0)
			imax = i - 1;
		else if(res > 0)
			imin = i + 1;
		else
			return i;
	}
	return HTML_ATTRIB_UNKNOWN;
}

HtmlTag html_lookup_length_tag(const char *string, size_t length) {
	//TODO: optimize string compare for binary tree search (no "restarts")
	int i, imin = 0, imax = HTML_TAGS, res;

	while(imax >= imin) {
		i = (imax - imin)/2 + imin;
		if (i==HTML_TAGS)
			break;
		res = stringcompare(string, html_tag[i], length);
		if(res < 0)
			imax = i - 1;
		else if(res > 0)
			imin = i + 1;
		else
			return i;
	}

	return HTML_TAG_UNKNOWN;
}

HtmlTag html_lookup_tag(const char *string) {
	int i, imin = 0, imax = HTML_TAGS, res;
	
	while(imax >= imin) {
		i = (imax - imin)/2 + imin;
		if (i==HTML_TAGS)
			break;
		res = stringcompare(string, html_tag[i], ~0);
		if(res < 0)
			imax = i - 1;
		else if(res > 0)
			imin = i + 1;
		else
			return i;
	}
	return HTML_TAG_UNKNOWN;
}

HtmlAttrib *html_new_element_attrib(enum HtmlAttribKey key, char *key_name, const char* value, size_t length) {
	HtmlAttrib *attrib;
	if(!(attrib = malloc(sizeof(HtmlAttrib))))
		return NULL;
	
	attrib->key = key;
	attrib->key_name = key_name;
	attrib->value = stringduplicate_length(value, length);
	attrib->next = NULL;
	
	return attrib;
}

HtmlElement *html_new_element(HtmlTag tag, char *tag_name, HtmlAttrib *attrib, HtmlElement *child, HtmlElement *sibling, char *text) {
	HtmlElement *elem;
	if(!(elem = malloc(sizeof(HtmlElement))))
		return NULL;
	
	elem->text = text;
	elem->tag = tag;
	elem->tag_name = tag_name;
	elem->attrib = attrib;
	elem->child = child;
	elem->sibling = sibling;
	
	return elem;
}

HtmlParseState *html_parse_begin() {
	HtmlParseState *state;
	if(!(state = malloc(sizeof(HtmlParseState))))
		return NULL;
	
	state->document = NULL;
	state->stack = NULL;
	state->elem = NULL;
	state->tag = 0;
	state->attrib_key = 0;
	state->state = STATE_CHILD;
	state->stringlen = 0;
	state->space = 0;
	
	if(!(state->document = malloc(sizeof(HtmlDocument))))
		goto error;
	if(!(state->elem = html_new_element(HTML_TAG_NONE, NULL, NULL, NULL, NULL, NULL)))
		goto error;
	if(!stack_push(&state->stack, state->elem))
		goto error;
	
	state->document->root_element = state->elem;
	state->elem = NULL;
	
	return state;
	
	error:
	free(state->document);
	free(state->elem);
	while(stack_pop(&state->stack));
	free(state);
	
	return NULL;
}

const char *html_parse_stream(HtmlParseState *state, const char *stream, const char *token, size_t len) {
	//TODO: support entities
	//TODO: support more xml bullcrap, like CDATA
	//TODO: clean up. remove code duplication (at least macrofy)
	
	#define ADVANCE_TOKEN token = stream; \
		state->stringlen = 0; \
		state->space = 0
	
	char c;
	char *text;
	
	HtmlElement *elem_tmp;
	HtmlAttrib *attrib_tmp;
	size_t i;
	
	if(!(state && stream && len))
		return NULL;
	
	for(i = 0; i < len; i++) {
		c = *stream++;
		reswitch:
		switch(state->state) {
			case STATE_CHILD:
				switch(c) {
					case '<':
						/* Add containing text */
						if(state->stringlen) {
							if(state->stringlen > 1 || !isspace(*token)) {
								text = stringduplicate_length(token, state->stringlen);
								
								if(!(elem_tmp = html_new_element(HTML_TAG_NONE, NULL, NULL, NULL, NULL, text)))
									goto error;
								if(state->elem) {
									state->elem->sibling = elem_tmp;
									state->elem = elem_tmp;
								} else {
									state->elem = stack_peek(&state->stack);
									state->elem->child = elem_tmp;
									state->elem = elem_tmp;
								}
							}
						}
						ADVANCE_TOKEN;
						state->state = STATE_OPEN;
						//tag = 0;
						continue;
					CASE_SPACE:
						if(html_tag_is_script(state->tag)) {
							ADVANCE_TOKEN;
						} else if(!state->space) {
							state->space = 1;
							state->stringlen++;
						}
						continue;
					default:
						if(html_tag_is_script(state->tag)) {
							ADVANCE_TOKEN;
						} else {
							state->space = 0;
							state->stringlen++;
						}
						continue;
				}
			case STATE_OPEN:
				if(html_tag_is_script(state->tag)) {
					/*I hate script tags*/
					if(c != '/') {
						ADVANCE_TOKEN;
						state->state = STATE_CHILD;
						continue;
					}
				}
				switch(c) {
					CASE_SPACE:
						ADVANCE_TOKEN;
						/*testing this*/
						//state = STATE_CHILD;
						continue;
					
					/*Comments, doctypes, xml-stuff and other crap we don't care about*/
					case '!':
						ADVANCE_TOKEN;
						state->state = STATE_DECLARATION;
						continue;
					case '?':
						state->state = STATE_BEGIN;
						continue;
					
					case '/':
						ADVANCE_TOKEN;
						state->state = STATE_END;
						continue;
					default:
						state->state = STATE_BEGIN;
						continue;
				}
			case STATE_DECLARATION:
				ADVANCE_TOKEN;
				switch(c) {
					case '-':
						state->state = STATE_COMMENT_BEGIN;
						continue;
					default:
						state->state = STATE_END_CLOSE;
						continue;
				}
			case STATE_COMMENT_BEGIN:
				ADVANCE_TOKEN;
				state->state = STATE_COMMENT;
				continue;
			case STATE_COMMENT:
			case STATE_COMMENT_END1:
				ADVANCE_TOKEN;
				switch(c) {
					case '-':
						state->state++;
						continue;
					default:
						state->state = STATE_COMMENT;
						continue;
				}
			case STATE_COMMENT_END2:
				ADVANCE_TOKEN;
				switch(c) {
					case '>':
						state->state = STATE_CHILD;
						continue;
					default:
						state->state = STATE_COMMENT;
						continue;
				}
			case STATE_BEGIN:
				switch(c) {
					CASE_SPACE:
						state->tag = html_lookup_length_tag(token, (stream - 1) - token);
						state->tag_name = stringduplicate_length(token, (stream - 1) - token);
						state->state = STATE_ATTRIB;
						ADVANCE_TOKEN;
						continue;
					case '>':
						state->tag = html_lookup_length_tag(token, (stream - 1) - token);
						state->tag_name = stringduplicate_length(token, (stream - 1) - token);
						state->state = STATE_CLOSE;
						ADVANCE_TOKEN;
						goto reswitch;
					case '/':
						state->tag = html_lookup_length_tag(token, (stream - 1) - token);
						state->tag_name = stringduplicate_length(token, (stream - 1) - token);
						state->state = STATE_SELFCLOSE;
						ADVANCE_TOKEN;
						continue;
					default:
						continue;
				}
			case STATE_ATTRIB:
				switch(c) {
					CASE_SPACE:
						ADVANCE_TOKEN;
						continue;
					case '/':
						state->state = STATE_SELFCLOSE;
						ADVANCE_TOKEN;
						continue;
					case '?':
						ADVANCE_TOKEN;
					case '>':
						state->state = STATE_CLOSE;
						ADVANCE_TOKEN;
						goto reswitch;
					default:
						state->state = STATE_ATTRIB_KEY;
						continue;
				}
			case STATE_ATTRIB_KEY:
				switch(c) {
					CASE_SPACE:
						//key key
						state->attrib_key = html_lookup_length_attrib_key(token, (stream - 1) - token);
						state->attrib_key_name = stringduplicate_length(token, (stream - 1) - token);
						attrib_tmp = html_new_element_attrib(state->attrib_key, state->attrib_key_name, NULL, 0);
						attrib_append(&state->attrib, attrib_tmp);
						attrib_tmp = NULL;
						
						state->state = STATE_ATTRIB;
						ADVANCE_TOKEN;
						continue;
					case '>':
						state->attrib_key = html_lookup_length_attrib_key(token, (stream - 1) - token);
						state->attrib_key_name = stringduplicate_length(token, (stream - 1) - token);
						attrib_tmp = html_new_element_attrib(state->attrib_key, state->attrib_key_name, NULL, 0);
						attrib_append(&state->attrib, attrib_tmp);
						attrib_tmp = NULL;
						
						state->state = STATE_CLOSE;
						ADVANCE_TOKEN;
						goto reswitch;
					case '/':
						state->attrib_key = html_lookup_length_attrib_key(token, (stream - 1) - token);
						state->attrib_key_name = stringduplicate_length(token, (stream - 1) - token);
						attrib_tmp = html_new_element_attrib(state->attrib_key, state->attrib_key_name, NULL, 0);
						attrib_append(&state->attrib, attrib_tmp);
						attrib_tmp = NULL;
						
						state->state = STATE_SELFCLOSE;
						ADVANCE_TOKEN;
						continue;
					case '=':
						state->attrib_key = html_lookup_length_attrib_key(token, (stream - 1) - token);
						state->attrib_key_name = stringduplicate_length(token, (stream - 1) - token);
						state->state = STATE_ATTRIB_VALUE;
						ADVANCE_TOKEN;
						continue;
					default:
						continue;
				}
			case STATE_ATTRIB_QUOTEVALUE:
				switch(c) {
					case '\'':
					case '"':
						attrib_tmp = html_new_element_attrib(state->attrib_key, state->attrib_key_name, token, (stream - 1) - token);
						attrib_append(&state->attrib, attrib_tmp);
						attrib_tmp = NULL;
						
						state->state = STATE_ATTRIB;
						ADVANCE_TOKEN;
						continue;
					default:
						continue;
				}
			case STATE_ATTRIB_VALUE:
				switch(c) {
					CASE_SPACE:
						attrib_tmp = html_new_element_attrib(state->attrib_key, state->attrib_key_name, token, (stream - 1) - token);
						attrib_append(&state->attrib, attrib_tmp);
						attrib_tmp = NULL;
						
						state->state = STATE_ATTRIB;
						ADVANCE_TOKEN;
						continue;
					case '>':
						attrib_tmp = html_new_element_attrib(state->attrib_key, state->attrib_key_name, token, (stream - 1) - token);
						attrib_append(&state->attrib, attrib_tmp);
						attrib_tmp = NULL;
						
						state->state = STATE_CLOSE;
						ADVANCE_TOKEN;
						goto reswitch;
					case '"':
					case '\'':
						state->state = STATE_ATTRIB_QUOTEVALUE;
						ADVANCE_TOKEN;
						continue;
					case '/':
						attrib_tmp = html_new_element_attrib(state->attrib_key, state->attrib_key_name, token, (stream - 1) - token);
						attrib_append(&state->attrib, attrib_tmp);
						attrib_tmp = NULL;
						
						state->state = STATE_SELFCLOSE;
						ADVANCE_TOKEN;
						continue;
					default:
						continue;
				}
			case STATE_CLOSE:
				switch(c) {
					case '>':
						ADVANCE_TOKEN;
						if(html_tag_is_script(state->tag)) {
							state->state = STATE_CHILD;
							continue;
						}
						//add to stack
						if(!(elem_tmp = html_new_element(state->tag, state->tag_name, state->attrib, NULL, NULL, NULL)))
							goto error;
						if(state->elem) {
							state->elem->sibling = elem_tmp;
							state->elem = elem_tmp;
						} else {
							state->elem = stack_peek(&state->stack);
							state->elem->child = elem_tmp;
							state->elem = elem_tmp;
						}
						
						if(!html_tag_is_selfclose(state->tag)) {
							stack_push(&state->stack, state->elem);
							state->elem = NULL;
						}
						state->tag = 0;
						state->tag_name = 0;
						state->attrib = 0;
						state->attrib_key_name = 0;
						
						state->state = STATE_CHILD;
						continue;
					default:
						ADVANCE_TOKEN;
						continue;
				}
			case STATE_SELFCLOSE:
				switch(c) {
					case '>':
						ADVANCE_TOKEN;
						if(html_tag_is_script(state->tag)) {
							state->state = STATE_CHILD;
							continue;
						}
						//add to stack
						if(!(elem_tmp = html_new_element(state->tag, state->tag_name, state->attrib, NULL, NULL, NULL)))
							goto error;
						if(state->elem) {
							state->elem->sibling = elem_tmp;
							state->elem = elem_tmp;
						} else {
							state->elem = stack_peek(&state->stack);
							state->elem->child = elem_tmp;
							state->elem = elem_tmp;
						}
						state->tag = 0;
						state->attrib = 0;
						state->attrib_key_name = 0;
						
						state->state = STATE_CHILD;
						continue;
					default:
						ADVANCE_TOKEN;
						continue;
				}
			case STATE_END:
				switch(c) {
					CASE_SPACE:
						//find tag to close
						if(html_tag_is_script(state->tag)) {
							if((stream - 1) - token > strlen(html_tag[state->tag]))
								state->state = STATE_CHILD;
							else if(html_lookup_length_tag(token, (stream - 1) - token) == state->tag) {
								state->tag = 0;
								state->state = STATE_END_CLOSE;
							} else
								state->state = STATE_CHILD;
							ADVANCE_TOKEN;
							continue;
						}
						if((state->tag = html_lookup_length_tag(token, (stream - 1) - token)) < 0)
							state->tag = state->elem->tag;
						
						ADVANCE_TOKEN;
						if(!stack_find(&state->stack, findtag, &state->tag)) {
							state->state = STATE_CHILD;
							continue;
						}
						
						do {
							/*check for null, broken pages*/
							elem_tmp = stack_pop(&state->stack);
						} while(elem_tmp->tag != state->tag);
						
						state->elem = elem_tmp;
						
						state->state = STATE_END_CLOSE;
						continue;
					case '>':
						//find tag to close
						if(html_tag_is_script(state->tag)) {
							if((stream - 1) - token > strlen(html_tag[state->tag]))
								state->state = STATE_CHILD;
							else if(html_lookup_length_tag(token, (stream - 1) - token) == state->tag) {
								state->tag = 0;
								state->state = STATE_CHILD;
							} else
								state->state = STATE_CHILD;
							ADVANCE_TOKEN;
							continue;
						}
						if((state->tag = html_lookup_length_tag(token, (stream - 1) - token)) < 0)
							state->tag = state->elem->tag;
						
						ADVANCE_TOKEN;
						if(!stack_find(&state->stack, findtag, &state->tag)) {
							state->state = STATE_CHILD;
							continue;
						}
						do {
							/*check for null, broken pages*/
							elem_tmp = stack_pop(&state->stack);
						} while(elem_tmp->tag != state->tag);
						
						state->elem = elem_tmp;
						state->tag = 0;
						state->attrib = 0;
						state->attrib_key_name = 0;
						
						state->state = STATE_CHILD;
						continue;
					default:
						continue;
				}
			case STATE_END_CLOSE:
				switch(c) {
					case '>':
						state->state = STATE_CHILD;
						ADVANCE_TOKEN;
						continue;
					default:
						continue;
				}
			case STATE_ENTITY:
			case STATES:
				break;
		}
	}
	return token;
	
	error:
	/*Handle malloc fail somehow*/
	return NULL;
	
	#undef ADVANCE_TOKEN
}

HtmlDocument *html_parse_end(HtmlParseState *state) {
	HtmlDocument *document;
	
	if(!state)
		return NULL;
	
	document = state->document;
	while(stack_pop(&state->stack));
	free(state);
	
	return document;
}

void *html_print_dom_element(HtmlElement *element, int level) {
	int i;
	HtmlElement *sibling;
	HtmlAttrib *attrib;

	if(!element)
		return NULL;
	while(element) {
		sibling = element->sibling;
		for(i = 0; i < level; i++)
			printf("\t");
		if(element->text) {
			if(element->tag == HTML_TAG_NONE)
				printf("text   : %s\n", stringtrim_l(element->text));
			else	
				printf("element %s: %s\n", html_tag[element->tag], stringtrim_l(element->text));
		} else {
			printf("element: %s ", html_tag[element->tag]);
			attrib = element->attrib;
			if (attrib) {
				printf("[ ");
				do{
					if (attrib->key == HTML_ATTRIB_UNKNOWN) {
						printf("%s=\"%s\" ", attrib->key_name, attrib->value);
					} else {
						printf("%s=\"%s\" ", html_attrib[attrib->key], attrib->value);
					}
					attrib = attrib->next;
				}while(attrib);
				printf("]");
			}
			printf("\n");
		}
		html_print_dom_element(element->child, level + 1);
		element = sibling;
	}
	return NULL;
}

void *html_print_dom(HtmlDocument *document) {
	html_print_dom_element(document->root_element, 0);
	return NULL;
}

void *html_free_attrib(HtmlAttrib *attrib) {
	if(!attrib)
		return NULL;
	html_free_attrib(attrib->next);
	attrib->key = HTML_ATTRIB_NONE;
	free(attrib->key_name);
	free(attrib->value);
	free(attrib->next);
	return NULL;
}

void *html_free_element(HtmlElement *element) {
	if(!element)
		return NULL;
	while(element) {
		html_free_attrib(element->attrib);
		if(element->tag_name) {
			free(element->tag_name);
		}
		if(element->text) {
			free(element->text);
		}
		html_free_element(element->child);
		free(element);
		element = element->sibling;
	}
	return NULL;
}

void *html_free_document(HtmlDocument *document) {
	if(!document)
		return NULL;
	html_free_element(document->root_element);
	free(document);
	return NULL;
}
