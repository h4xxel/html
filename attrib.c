/*
 * html - a simple html parser lacking a better name 
 * The contents of this file is licensed under the MIT License,
 * see the file COPYING or http://opensource.org/licenses/MIT
 */

#include <stdlib.h>
#include <string.h>
#include "attrib.h"

/*keep sorted, binary search*/
const char *html_attrib[HTML_ATTRIB_KEYS] = {
	[HTML_ATTRIB_NONE] = "",
	
	[HTML_ATTRIB_UNKNOWN] = "",
	
	[HTML_ATTRIB_ABBR] = "abbr",
	[HTML_ATTRIB_ACCEPT_CHARSET] = "accept-charset",
	[HTML_ATTRIB_ACCEPT] = "accept",
	[HTML_ATTRIB_ACCESSKEY] = "accesskey",
	[HTML_ATTRIB_ACTION] = "action",
	[HTML_ATTRIB_ALIGN] = "align",
	[HTML_ATTRIB_ALT] = "alt",
	[HTML_ATTRIB_AXIS] = "axis",
	
	[HTML_ATTRIB_BORDER] = "border",
	
	[HTML_ATTRIB_CELLPADDING] = "cellpadding",
	[HTML_ATTRIB_CELLSPACING] = "cellspacing",
	[HTML_ATTRIB_CHAR] = "char",
	[HTML_ATTRIB_CHAROFF] = "charoff",
	[HTML_ATTRIB_CHARSET] = "charset",
	[HTML_ATTRIB_CHECKED] = "checked",
	[HTML_ATTRIB_CITE] = "cite",
	[HTML_ATTRIB_CLASS] = "class",
	[HTML_ATTRIB_CODETYPE] = "codetype",
	[HTML_ATTRIB_COLS] = "cols",
	[HTML_ATTRIB_COLSPAN] = "colspan",
	[HTML_ATTRIB_CONTENT] = "content",
	[HTML_ATTRIB_COORDS] = "coords",
	
	[HTML_ATTRIB_DATA] = "data",
	[HTML_ATTRIB_DATETIME] = "datetime",
	[HTML_ATTRIB_DECLARE] = "declare",
	[HTML_ATTRIB_DEFER] = "defer",
	[HTML_ATTRIB_DIR] = "dir",
	[HTML_ATTRIB_DISABLED] = "disabled",
	
	[HTML_ATTRIB_ENCTYPE] = "enctype",
	
	[HTML_ATTRIB_FACE] = "face",
	[HTML_ATTRIB_FOR] = "for",
	[HTML_ATTRIB_FRAME] = "frame",
	[HTML_ATTRIB_FRAMEBORDER] = "frameborder",
	
	[HTML_ATTRIB_HEADERS] = "headers",
	[HTML_ATTRIB_HEIGHT] = "height",
	[HTML_ATTRIB_HREF] = "href",
	[HTML_ATTRIB_HREFLANG] = "hreflang",
	[HTML_ATTRIB_HSPACE] = "hspace",
	[HTML_ATTRIB_HTTP_EQUIV] = "http-equiv",
	
	[HTML_ATTRIB_ID] = "id",
	[HTML_ATTRIB_ISMAP] = "ismap",
	
	[HTML_ATTRIB_LABEL] = "label",
	[HTML_ATTRIB_LANG] = "lang",
	[HTML_ATTRIB_LONGDESC] = "longdesc",
	
	[HTML_ATTRIB_MARGINHEIGHT] = "marginheight",
	[HTML_ATTRIB_MARGINWIDTH] = "marginwidth",
	[HTML_ATTRIB_MAXLENGTH] = "maxlength",
	[HTML_ATTRIB_MEDIA] = "media",
	[HTML_ATTRIB_METHOD] = "method",
	[HTML_ATTRIB_MULTIPLE] = "multiple",
	
	[HTML_ATTRIB_NAME] = "name",
	[HTML_ATTRIB_NOHREF] = "noref",
	[HTML_ATTRIB_NORESIZE] = "noresize",
	
	[HTML_ATTRIB_ONBLUR] = "onblur",
	[HTML_ATTRIB_ONCHANGE] = "onchange",
	[HTML_ATTRIB_ONCLICK] = "onclick",
	[HTML_ATTRIB_ONDBLCLICK] = "ondblclick",
	[HTML_ATTRIB_ONFOCUS] = "onfocus",
	[HTML_ATTRIB_ONKEYDOWN] = "onkeydown",
	[HTML_ATTRIB_ONKEYPRESS] = "onkeypress",
	[HTML_ATTRIB_ONKEYUP] = "onkeyup",
	[HTML_ATTRIB_ONLOAD] = "onload",
	[HTML_ATTRIB_ONMOUSEDOWN] = "onmousedown",
	[HTML_ATTRIB_ONMOUSEMOVE] = "onmousemove",
	[HTML_ATTRIB_ONMOUSEOUT] = "onmouseout",
	[HTML_ATTRIB_ONMOUSEOVER] = "onmouseover",
	[HTML_ATTRIB_ONMOUSEUP] = "onmouseup",
	[HTML_ATTRIB_ONRESET] = "onreset",
	[HTML_ATTRIB_ONSELECT] = "onselect",
	[HTML_ATTRIB_ONSUBMIT] = "onsubmit",
	[HTML_ATTRIB_ONUNLOAD] = "onunload",
	
	[HTML_ATTRIB_PROFILE] = "profile",
	
	[HTML_ATTRIB_READONLY] = "readonly",
	[HTML_ATTRIB_REL] = "rel",
	[HTML_ATTRIB_REV] = "rev",
	[HTML_ATTRIB_ROWS] = "rows",
	[HTML_ATTRIB_ROWSPAN] = "rowspan",
	[HTML_ATTRIB_RULES] = "rules",
	
	[HTML_ATTRIB_SCHEME] = "scheme",
	[HTML_ATTRIB_SCOPE] = "scope",
	[HTML_ATTRIB_SCROLLING] = "scrolling",
	[HTML_ATTRIB_SELECTED] = "selected",
	[HTML_ATTRIB_SHAPE] = "shape",
	[HTML_ATTRIB_SIZE] = "size",
	[HTML_ATTRIB_SPAN] = "span",
	[HTML_ATTRIB_SRC] = "src",
	[HTML_ATTRIB_STANDBY] = "standby",
	[HTML_ATTRIB_STYLE] = "style",
	[HTML_ATTRIB_SUMMARY] = "summary",
	
	[HTML_ATTRIB_TABINDEX] = "tabindex",
	[HTML_ATTRIB_TARGET] = "target",
	[HTML_ATTRIB_TITLE] = "title",
	[HTML_ATTRIB_TYPE] = "type",
	
	[HTML_ATTRIB_USEMAP] = "usemap",
	
	[HTML_ATTRIB_VALIGN] = "valign",
	[HTML_ATTRIB_VALUE] = "value",
	[HTML_ATTRIB_VALUETYPE] = "valuetype",
	
	[HTML_ATTRIB_WIDTH] = "width",

	[HTML_ATTRIB_XML__LANG] = "xml:lang",
	[HTML_ATTRIB_XMLNS] = "xmlns",
};

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
