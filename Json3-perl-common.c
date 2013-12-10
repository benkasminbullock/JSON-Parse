/* These things are common between the validation and the parsing
   routines. This is #included into "Json3.xs". */

/* Match whitespace. Whitespace is as defined by the JSON standard,
   not by Perl. */

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

/* Match digits. */

#define DIGIT19 \
    '1':	\
 case '2':	\
 case '3':	\
 case '4':	\
 case '5':	\
 case '6':	\
 case '7':	\
 case '8':	\
 case '9'

/* A "string_t" is a pointer into the input, which lives in
   "parser->input". The "string_t" structure is used for copying
   strings when the string does not contain any escapes. When a string
   contains escapes, it is copied into "parser->buffer". */

typedef struct string {

    char * start;
    unsigned int length;

    /* The "bad_boys" flag is set if there are backslash escapes in
       the string like "\r", so that it needs to be cleaned up before
       using it. That means we use "parser->buffer". This is to speed
       things up, by not doing the cleanup when it isn't necessary. */

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

    /* Force unicode. This happens when we hit "\uxyzy". */

    unsigned int force_unicode : 1;
}
parser_t;

#ifdef __GNUC__
#define INLINE inline
#else
#define INLINE
#endif /* def __GNUC__ */

/* All instances of JSON literals are pointed to the following. */

static SV * json_true;
static SV * json_false;
static SV * json_null;
static SV * empty_string;

/* The size of the buffer for printing errors. */

#define BURGERSIZE 0x1000

/* Error fallthrough. This takes the error and sends it to "croak". */

static INLINE void failburger (parser_t * parser, const char * format, ...)
{
    char buffer[BURGERSIZE];
    va_list a;
    va_start (a, format);
    vsnprintf (buffer, BURGERSIZE, format, a);
    va_end (a);
    croak ("Line %d, byte %d/%d: %s", parser->line,
	   parser->end - parser->input,
	   parser->length, buffer);
}

#undef BURGERSIZE

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
	if (! parser->buffer) {
	    failburger (parser, "out of memory");
	}
    }
}

static INLINE char *
do_unicode_escape (parser_t * parser, char * p, char ** b_ptr)
{
    int k;
    char unibuf[5];
    int unicode;
    int plus;
    for (k = 0; k < strlen ("ABCD"); k++) {
	unibuf[k] = *((p)++);
    }
    unibuf[4] = '\0';
    unicode = strtol (unibuf, 0, 16);
    plus = ucs2_to_utf8 (unicode, *b_ptr);
    if (plus == UNICODE_BAD_INPUT) {
	failburger (parser,
		    "bad unicode escape "
		    "'\\u%s'",
		    unibuf);
    }
    else if (plus == UNICODE_SURROGATE_PAIR) {
	int unicode2;
	int plus2;
	if (p[0] == '\\' && p[1] == 'u') {
	    for (k = 0; k < strlen ("ABCD"); k++) {
		unibuf[k] = p[k + 2];
	    }
	    unibuf[4] = '\0';
	    unicode2 = strtol (unibuf, 0, 16);
	    plus2 = surrogate_to_utf8 (unicode, unicode2, * b_ptr);
	    if (plus2 <= 0) {
		failburger (parser, "surrogate pair unreadable");
	    }
	    p += 6;
	    * b_ptr += plus2;
	    return p;
	}
	else {
	    failburger (parser, "second half of surrogate pair not found");
	}
    }
    else if (plus <= 0) {
	failburger (parser, "error decoding \\u%s\n", unibuf);
    }
    * b_ptr += plus;
    return p;
}

/* Handle backslash escapes. */

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
	p = do_unicode_escape (parser, p, & b);		\
	if (! parser->unicode) {			\
	    parser->force_unicode = 1;			\
	}						\
	break;						\
							\
    default:						\
	failburger (parser, "Unknown escape \\%c", c);	\
    }

/* Resolve "s" by converting escapes into the appropriate things. Put
   the result into "parser->buffer". The return value is the length of
   the string. */

static INLINE int
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

static INLINE void
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

static INLINE int
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

static void
parser_free (parser_t * parser)
{
    if (parser->buffer) {
	free (parser->buffer);
    }
}

