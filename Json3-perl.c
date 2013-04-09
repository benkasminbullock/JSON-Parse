/* Match whitespace. */

#define WHITESPACE         \
    '\n':                  \
    parser->line++;	   \
    /* Fallthrough. */	   \
 case ' ':                 \
 case '\t':                \
 case '\r'

/* Match digits. */

#define DIGIT \
    '0':      \
 case '1':    \
 case '2':    \
 case '3':    \
 case '4':    \
 case '5':    \
 case '6':    \
 case '7':    \
 case '8':    \
 case '9'

/* A "string_t" is a pointer into the input, which lives in
   "parser->input". The "string_t" structure is used for copying
   strings when the string does not contain any escapes. When a string
   contains escapes, it is copied into "parser->buffer". */

typedef struct string {

    char * start;
    unsigned int length;

    /* Flag is set if there are escapes in the string like \r, so that
       it needs to be cleaned up before using it. That means we use
       "parser->buffer". */

    unsigned bad_boys : 1;
}
string_t;

typedef struct parser {

    /* The length of "input". */

    unsigned int length;

    /* The input. */

    char * input;

    /* The end-point of the parsing. This increments through
       "input". */

    char * end;

    /* The last byte of "input", "parser->input +
       parser->length". This is used to detect overflows. */

    char * last_byte;

    /* Allocated size of "buffer". */

    int buffer_size;

    /* Buffer to stick strings into temporarily. */

    char * buffer;

    /* Line number. */

    int line;

    /* Unicode? */

    unsigned int unicode : 1;
    
}
parser_t;

/* JSON literals are all pointed to these bananas. */

static SV * json_true;
static SV * json_false;
static SV * json_null;
static SV * empty_string;

/* Error fallthrough. This takes the error and sends it to "croak". */

static inline void failburger (parser_t * parser, const char * format, ...)
{
    char buffer[0x1000];
    va_list a;
    va_start (a, format);
    vsnprintf (buffer, 0x1000, format, a);
    va_end (a);
    croak ("Line %d, byte %d/%d: %s", parser->line,
	   parser->end - parser->input,
	   parser->length, buffer);
}

/*#define INT_MAX_DIGITS ((int) (log (INT_MAX) / log (10)) - 1)*/
#define INT_MAX_DIGITS 8

/* Turn a number into an SV. */

static inline SV *
number (parser_t * parser)
{
    /* Various parsing flags. */

    /* Set to 1 if we saw ".". */

    int dot;

    /* Set to 1 if we saw "e" or "E". */

    int exp;

    /* Set to 1 if we saw "+". */

    int plus;

    /* Set to 1 if we saw a leading zero. */

    int zero;

    /* Set to 1 if we saw a leading "-". */

    int minus;

    /* Set to 1 if we saw a "-" after we saw "e" or "E". */

    int expminus;

    /* End marker for strtod/strtol. */

    char * end;

    /* Start marker for strtod/strtol. */

    char * start;

    /* A guess for integer numbers. */

    int guess;

    /* The axis of our motion. */

    char c;

    /* Set all the flags to zero. */

    dot = 0;
    exp = 0;
    plus = 0;
    zero = 0;
    minus = 0;
    expminus = 0;
    guess = 0;

    parser->end--;
    start = parser->end;

 number_start:

    switch (c = *parser->end++) {

    case '.':

	if (dot) {
	    failburger (parser, "Too many decimal points");
	}
	dot = 1;
	goto number_start;

    case '+':

	if (! exp) {
	    failburger (parser, "Plus outside exponential");
	}
	if (plus) {
	    failburger (parser, "Double plus ungood");
	}
	plus = 1;
	goto number_start;
	
    case '-':

	if (exp) {
	    if (expminus) {
		failburger (parser, "Double minus in exponent");
	    }
	    expminus = 1;
	    goto number_start;
	}
	else {
	    if (minus) {
		failburger (parser, "Double minus");
	    }
	    minus = 1;
	    goto number_start;
	}
	
    case 'e':
    case 'E':

	if (exp) {
	    failburger (parser, "Doubled exponential");
	}
	exp = 1;
	goto number_start;

    case DIGIT:

	if (! dot && ! exp) {
	    guess = 10 * guess + (c - '0');
	}
	goto number_start;

    default:

	parser->end--;
	break;
    }

    if (dot || exp) {

	/* This is a floating point number, so we sent it to
	   "strtod". */

	double d;

	d = strtod (start, & end);
	if (end == parser->end) {
	    return newSVnv (d);
	}
    }
    else {
	if (parser->end - start < INT_MAX_DIGITS + minus) {
	    if (minus) {
		guess = -guess;
	    }
	    return newSViv (guess);
	}
	else {
	    int i;
	    
	    i = strtol (start, & end, 10);
	    if (end == parser->end) {
		return newSViv (i);
	    }
	}
    }

    /* Convert to a string. */

    return newSVpv (start, parser->end - start);
}

/* Get more memory for "parser->buffer". */

static void
expand_buffer (parser_t * parser, int length)
{
    if (parser->buffer_size < 2 * length + 0x100) {
	parser->buffer_size = 2 * length + 0x100;
	if (parser->buffer) {
	    parser->buffer = realloc (parser->buffer, parser->buffer_size);
	}
	else {
	    parser->buffer = malloc (parser->buffer_size);
	}
    }
}

#define HANDLE_ESCAPES(p)				\
    switch (c = * ((p)++)) {				\
							\
    case '\\':						\
    case '/':						\
    case '"':						\
	*b++ = c;					\
	break;						\
							\
    case 'b':						\
	*b++ = '\b';					\
	break;						\
							\
    case 'f':						\
	*b++ = '\f';					\
	break;						\
							\
    case 'n':						\
	*b++ = '\n';					\
	break;						\
							\
    case 'r':						\
	*b++ = '\r';					\
	break;						\
							\
    case 't':						\
	*b++ = '\t';					\
	break;						\
							\
    case 'u':						\
	if (parser->unicode) {				\
							\
	}						\
	else {						\
	    /* Copy it. */				\
	    int k;					\
	    *b++ = '\\';				\
	    *b++ = c;					\
	    for (k = 0; k < strlen ("ABCD"); k++) {	\
		*b++ = *((p)++);			\
	    }						\
	}						\
	break;						\
							\
    default:						\
	failburger (parser, "Unknown escape \\%c", c);	\
    }

/* Resolve "s" by converting escapes into the appropriate things. Put
   the result into "parser->buffer". The return value is the length of
   the string. */

static int
resolve_string (parser_t * parser, string_t * s)
{
    /* The pointer where we copy the string. This points into into
       "parser->buffer". */

    char * b;

    /* The pointer into "parser->input", using "s->start" to get the
       start point. We don't use "parser->end" for this job because
       "resolve_string" is called only after the value of the object
       is resolved. E.g. if the object goes like

       {"hot":{"potatoes":"tomatoes"}}

       then this routine is called first for "potatoes" and then for
       "hot" as each sub-element of the hashes is resolved. We don't
       want to mess around with the value of "parser->end", which is
       always pointing to one after the last byte viewed. */

    char * p;

    p = s->start;

    /* Ensure we have enough memory to fit the string. */

    expand_buffer (parser, s->length);

    b = parser->buffer;

    while (p - s->start < s->length) {
	char c;

	c = *p++;
	if (c == '\\') {
	    HANDLE_ESCAPES(p);
	}
	else {
	    *b++ = c;
	}
    }

    /* This is the length of the string in bytes. */

    return b - parser->buffer;
}

/* Get an object key value and put it into "key". Check for
   escapes. */

static inline void
get_key_string (parser_t * parser, string_t * key)
{
    char c;
    key->start = parser->end;
    key->bad_boys = 0;
    while ((c = *parser->end++)) {

	/* Go on eating bytes until we find a ". */

	if (c == '"') {
	    break;
	}
	if (parser->end >= parser->last_byte) {
	    failburger (parser, "Object key string went past end");
	}

	/* Skip over \x, where x is anything at all. This includes \"
	   of course. */

	if (c == '\\') {
	    key->bad_boys = 1;
	    parser->end++;
	}
    }
    key->length = parser->end - key->start - 1;
}

/* Resolve the string pointed to by "parser->end" into
   "parser->buffer". The return value is the length of the
   string. This is only called if the string has \ escapes in it. */

static inline int
get_string (parser_t * parser)
{
    char * b;
    char c;

    if (! parser->buffer) {
	expand_buffer (parser, 0x1000);
    } 
    b = parser->buffer;
    while ((c = *parser->end++)) {
	switch (c) {
	case '"':
	    goto string_end;
	    break;
	case '\\':
	    HANDLE_ESCAPES(parser->end);
	    break;
	case '\0':
	    failburger (parser, "Null byte in string");
	    break;
	default:
	    * b++ = c;
	    break;
	}
	if (b - parser->buffer >= parser->buffer_size - 0x100) {
	    /* Save our offset in parser->buffer, because "realloc" is
	       called by "expand_buffer", and "b" may no longer point
	       to a meaningful location. */
	    int size = b - parser->buffer;
	    expand_buffer (parser, 2 * parser->buffer_size);
	    b = parser->buffer + size;
	}
	if (parser->end >= parser->last_byte) {
	    failburger (parser, "Object key string went past end");
	}
    }
 string_end:
    return b - parser->buffer;
}

static SV *
string (parser_t * parser)
{
    char c;
    SV * string;
    int len;
    char * start;

    start = parser->end;
    len = 0;

    /* First of all, we examine the string to work out how long it is
       and to look for escapes. If we find them, we go to "bad_boys"
       and go back and do all the hard work of converting the escapes
       into the right things. If we don't find any escapes, we just
       use "start" and "len" and copy the string from inside
       "input". */

    while ((c = *parser->end++)) {
	switch (c) {
	case '"':
	    goto string_end;
	case '\\':
	    goto bad_boys;
	default:
	    len++;
	}
    }
    failburger (parser, "Null byte while reading string");

 string_end:

    string = newSVpvn (start, len);
    return string;

 bad_boys:

    parser->end = start;

    len = get_string (parser);

    string = newSVpvn (parser->buffer, len);

    return string;
}

/* JSON literals, a complete nuisance for people writing JSON
   parsers. */

static SV *
literal (parser_t * parser, char c)
{
    switch (c) {
    case 't':
	if (* parser->end++ == 'r'
	    &&
	    * parser->end++ == 'u'
	    &&
	    * parser->end++ == 'e') {
	    SvREFCNT_inc (json_true);
	    return json_true;
	}
	break;

    case 'n':
	if (* parser->end++ == 'u'
	    &&
	    * parser->end++ == 'l'
	    &&
	    * parser->end++ == 'l') {
	    SvREFCNT_inc (json_null);
	    return json_null;
	}
	break;

    case 'f':
	if (* parser->end++ == 'a'
	    &&
	    * parser->end++ == 'l'
	    &&
	    * parser->end++ == 's'
	    &&
	    * parser->end++ == 'e') {
	    SvREFCNT_inc (json_false);
	    return json_false;
	}
	break;

    default:
	failburger (parser, "Whacko attempt to make a literal starting with %c",
		    c); 
    }
    failburger (parser, "Unparseable character %c in literal",
		parser->end - 1); 

    /* Unreached, shut up compiler warnings. */

    return & PL_sv_undef;
}

static SV * object (parser_t * parser);

/* This goes in the switch statement in both "object ()" and "array
   ()". */

#define PARSE(start)				\
						\
 case WHITESPACE:				\
 goto start;					\
						\
 case '"':					\
 value = string (parser);			\
 break;						\
						\
 case '-':					\
 case DIGIT:					\
 value = number (parser);			\
 break;						\
						\
 case '{':					\
 value = object (parser);			\
 break;						\
						\
 case '[':					\
 value = array (parser);			\
 break;						\
						\
 case 'f':					\
 case 'n':					\
 case 't':					\
 value = literal (parser, c);			\
 break


/* We have seen "[", so now deal with the contents of an array. At the
   end of this routine, "parser->end" is pointing one beyond the final
   "]" of the array. */

static SV *
array (parser_t * parser)
{
    char c;
    AV * av;
    int middle;
    SV * value;

    middle = 0;
    av = newAV ();

 array_start:

    switch (c = *parser->end++) {

	PARSE(array_start);

    case ']':

	goto array_end;

    case ',':

	if (middle) {
	    goto array_start;
	}

    default:
	failburger (parser, "unknown character in array '%c'", c);
    }

    middle = 1;
    av_push (av, value);
    goto array_start;

 array_end:

    return newRV_noinc ((SV *) av);
}

/* We have seen "{", so now deal with the contents of an object. At
   the end of this routine, "parser->end" is pointing one beyond the
   final "}" of the object. */

static SV *
object (parser_t * parser)
{
    char c;
    HV * hv;
    string_t key;
    SV * value;
    int middle;

    middle = 0;
    hv = newHV ();

 hash_start:

    switch (c = *parser->end++) {

    case WHITESPACE:
	goto hash_start;

    case '}':
	goto hash_end;

    case '"':
	get_key_string (parser, & key);
	goto hash_next;

    case ',':
	if (middle) {
	    goto hash_start;
	}

    default:
	failburger (parser, "Unknown character '%c' in object key", c);
    }

 hash_next:

    switch (*parser->end++) {

    case WHITESPACE:
	goto hash_next;

    case ':':
	middle = 1;
	goto hash_value;

    default:
	failburger (parser, "Unknown character '%c' after object key", c);
    }

 hash_value:

    switch (c = * parser->end++) {

	PARSE(hash_value);

    default:
	failburger (parser, "Unknown character '%c' in object value", c);
    }
    if (key.bad_boys) {
	int klen;

	klen = resolve_string (parser, & key);
	(void) hv_store (hv, parser->buffer, klen, value, 0);
    }
    else {
	(void) hv_store (hv, key.start, key.length, value, 0);
    }

    goto hash_start;

 hash_end:

    return newRV_noinc ((SV *) hv);
}

static void
parser_free (parser_t * parser)
{
    if (parser->buffer) {
	free (parser->buffer);
    }
}

/* This is the entry point for the routine. */

static SV *
parse (SV * json)
{
    /* The axis of our motions. */

    char c;

    /* Our collection of bits and pieces. */

    parser_t parser_o = {0};

    /* The returned object. */

    SV * r;

    /* Set up the object. */

    parser_o.line = 1;

    parser_o.end = parser_o.input = SvPV (json, parser_o.length);
    parser_o.last_byte = parser_o.input + parser_o.length;
    parser_o.unicode = SvUTF8 (json);

 parse_start:

    switch (c = *parser_o.end++) {

    case '{':
	r = object (& parser_o);
	break;

    case '[':
	r = array (& parser_o);
	break;

	/* Whitespace. */

    case '\n':
	parser_o.line++;

	/* Fallthrough. */

    case ' ':
    case '\t':
    case '\r':
	goto parse_start;

    default:
	failburger (& parser_o, "Bad character %c in initial state", c);
    }

    parser_free (& parser_o);

    return r;
}

