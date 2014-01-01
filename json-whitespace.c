/* Type for adding whitespace. */

typedef struct json_ws {
    SV * news;
    SV * olds;
    unsigned int news_l;
    /* Copy point. */
    char * q;
    /* Top of token tree. */
    json_token_t * t;
    /* Token under examination now. */
    json_token_t * next;
}
json_ws_t;

static int copy_json (char * p, char * q, json_token_t * t)
{
    json_token_t * next;
    char * c;
    next = t;
    char * qorig;
    qorig = q;
    while (next) {
	switch (next->type) {
	case json_token_object:
	    *q++ = '{';
	    q += copy_json (p, q, next->child);
	    *q++ = '}';
	    break;
	case json_token_array:
	    *q++ = '[';
	    q += copy_json (p, q, next->child);
	    *q++ = ']';
	    break;
	case json_token_string:
	case json_token_key:
	case json_token_literal:
	case json_token_number:
	    for (c = p + next->start; c <= p + next->end; c++) {
		*q++ = *c;
	    }
	    break;
	case json_token_comma:
	    *q++ = ',';
	    break;
	case json_token_colon:
	    *q++ = ':';
	    break;
	default:
	    croak ("unhandled token type %d", next->type);
	}
	next = next->next;
    }
    return q - qorig;
}

static SV * strip_whitespace (json_token_t * tokens, SV * json)
{
    SV * stripped;
    char * p;
    char * q;
    unsigned int l;
    unsigned int m;
    p = SvPV (json, l);
    stripped = newSV (l);
    /* Tell Perl it's a string. */
    SvPOK_on (stripped);
    /* Set UTF-8 if necessary. */
    if (SvUTF8 (json)) {
	SvUTF8_on (stripped);
    }
    /* Get a pointer to the string inside "stripped". */
    q = SvPVX (stripped);
    m = copy_json (p, q, tokens);
    /* Set the length. */
    SvCUR_set (stripped, m);
    return stripped;
}
