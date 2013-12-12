/* These things are common between the validation and the parsing
   routines. This is #included into "Json3.xs". */

/* The following matches bytes which are not allowed in JSON
   strings. "All Unicode characters may be placed within the quotation
   marks except for the characters that must be escaped: quotation
   mark, reverse solidus, and the control characters (U+0000 through
   U+001F)." - from section 2.5 of RFC 4627 */

#define BADBYTES				\
      '\0':case 0x01:case 0x02:case 0x03:	\
 case 0x04:case 0x05:case 0x06:case 0x07:	\
 case 0x08:case 0x09:case 0x0A:case 0x0B:	\
 case 0x0C:case 0x0D:case 0x0E:case 0x0F:	\
 case 0x10:case 0x11:case 0x12:case 0x13:	\
 case 0x14:case 0x15:case 0x16:case 0x17:	\
 case 0x18:case 0x19:case 0x1A:case 0x1B:	\
 case 0x1C:case 0x1D:case 0x1E:case 0x1F

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

/* This is a test for whether the string has ended, which we use when
   we catch a zero byte in an unexpected part of the input. */

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

    unsigned char * buffer;

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

/* The following are used in parsing \u escapes only. */

#define LHEXBYTE						     \
      'a': case 'b': case 'c': case 'd': case 'e': case 'f'

#define UHEXBYTE						     \
      'A': case 'B': case 'C': case 'D': case 'E': case 'F' 

/* Parse the hex bit of a \uXYZA escape. */

static INLINE int
parse_hex_bytes (parser_t * parser, char * p)
{
    int k;
    int unicode;

    unicode = 0;

    for (k = 0; k < strlen ("ABCD"); k++) {

	unsigned char c;

	c = p[k];

	switch (c) {

	case DIGIT:
	    unicode = unicode * 16 + c - '0';
	    break;

	case UHEXBYTE:
	    unicode = unicode * 16 + c - 'A' + 10;
	    break;

	case LHEXBYTE:
	    unicode = unicode * 16 + c - 'a' + 10;
	    break;

	case '\0':
	    if (p + k - parser->input >= parser->length) {
		failburger (parser, "Unexpected end of input parsing unicode escape");
	    }

	    /* Fallthrough */

	default:
	    failburger (parser,
			"Non-hexadecimal character '%c' parsing \\u escape",
			c);
	}
    }
    return unicode;
}

#undef LHEXBYTE
#undef UHEXBYTE

static INLINE char *
do_unicode_escape (parser_t * parser, char * p, unsigned char ** b_ptr)
{
    int unicode;
    unsigned int plus;
    unicode = parse_hex_bytes (parser, p);
    p += 4;
    plus = ucs2_to_utf8 (unicode, *b_ptr);
    if (plus == UNICODE_BAD_INPUT) {
	failburger (parser,
		    "bad unicode escape");
    }
    else if (plus == UNICODE_SURROGATE_PAIR) {
	int unicode2;
	int plus2;
	unsigned char c;
	if (*p++ == '\\' && *p++ == 'u') {
	    unicode2 = parse_hex_bytes (parser, p);
	    p += 4;
	    plus2 = surrogate_to_utf8 (unicode, unicode2, * b_ptr);
	    if (plus2 <= 0) {
		failburger (parser, "surrogate pair unreadable");
	    }
	    * b_ptr += plus2;
	    goto end;
	}
	else {
	    failburger (parser, "second half of surrogate pair not found");
	}
    }
    else if (plus <= 0) {
	failburger (parser, "error decoding unicode escape");
    }
    * b_ptr += plus;
 end:
    if (unicode >= 0x80 && ! parser->unicode) {
	parser->force_unicode = 1;
    }
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
	break;						\
							\
    default:						\
	failburger (parser,				\
		    "Unknown escape '\\%c'", c);	\
    }

/* Resolve "s" by converting escapes into the appropriate things. Put
   the result into "parser->buffer". The return value is the length of
   the string. */

static INLINE int
resolve_string (parser_t * parser, string_t * s)
{
    /* The pointer where we copy the string. This points into
       "parser->buffer". */

    unsigned char * b;

    /* "p" is the pointer into "parser->input", using "s->start" to
       get the start point. We don't use "parser->end" for this job
       because "resolve_string" is called only after the value of the
       object is resolved. E.g. if the object goes like

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

#define NEXTBYTE c = *parser->end++

/* Get an object key value and put it into "key". Check for
   escapes. */

static INLINE void
get_key_string (parser_t * parser, string_t * key)
{
    char c;
    key->start = parser->end;
    key->bad_boys = 0;
    while ((NEXTBYTE)) {

	/* Go on eating bytes until we find a ". */

	if (c == '"') {
	    break;
	}
	if (parser->end >= parser->last_byte) {
	    failburger (parser, "Unexpected end of input parsing object key string");
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

#define ILLEGALBYTE  \
    failburger (parser, "Illegal byte value '0x%02X' in string", c)


/* Resolve the string pointed to by "parser->end" into
   "parser->buffer". The return value is the length of the
   string. This is only called if the string has \ escapes in it. */

static INLINE int
get_string (parser_t * parser)
{
    unsigned char * b;
    unsigned char c;

    if (! parser->buffer) {
	expand_buffer (parser, 0x1000);
    }
    b = parser->buffer;
    while ((NEXTBYTE)) {
	switch (c) {

	case '"':
	    goto string_end;
	    break;

	case '\\':
	    HANDLE_ESCAPES(parser->end);
	    break;

	case BADBYTES:
	    ILLEGALBYTE;
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

#define STRINGEND (parser->end - parser->input >= parser->length)

