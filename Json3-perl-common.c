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

typedef enum {
    json_invalid,
    json_string,
    json_number,
    json_literal,
    json_object,
    json_array,
    json_unicode_escape,
    json_overflow
}
json_type_t;

const char * type_names[json_overflow] = {
    "invalid",
    "string",
    "number",
    "literal",
    "object",
    "array",
    "unicode escape"
};

typedef enum {
    json_error_invalid,
    json_error_unexpected_character,
    json_error_unexpected_end_of_input,
    json_error_non_hexadecimal_character,
    json_error_stray_comma,
    json_error_missing_comma,
    json_error_illegal_byte,
    json_error_bad_literal,
    json_error_stray_final_character,
    json_error_trailing_comma,
    json_error_too_many_decimal_points,
    json_error_leading_zero,
    json_error_second_half_of_surrogate_pair_missing,
    json_error_surrogate_pair_unreadable,
    json_error_unknown_escape,
    json_error_empty_input,
    json_error_overflow
}
json_error_t;

const char * json_errors[json_error_overflow] = {
    "invalid",
    "Unexpected character '%c'",
    "Unexpected end of input",
    "Non-hexadecimal character '%c'",
    "Stray comma",
    "Missing comma",
    "Illegal byte in string 0x%02x",
    "Unexpected character '%c' in literal",
    "Stray final character",
    "Trailing comma",
    "Too many decimal points",
    "Leading zero",
    "Second half of surrogate pair missing",
    "Surrogate pair unreadable",
    "Unknown escape '\\%c'",
    "Empty input",
};

enum expectation {
    xwhitespace,
    xvalue_start,
    xcomma,
    xvalue_separator,
    xobject_end,
    xarray_end,
    xhexadecimal_character,
    xstring_start,
    xunicode_escape,
    xdigit,
    xdot,
    xminus,
    n_expectations
};

#define XWHITESPACE (1<<xwhitespace)
#define VALUE_START (1<<xvalue_start)
#define COMMA (1<<xcomma)
#define VALUE_SEPARATOR (1<<xvalue_separator)
#define OBJECT_END (1<<xobject_end)
#define ARRAY_END (1<<xarray_end)
#define HEXADECIMAL_CHARACTER (1<<xhexadecimal_character)
#define STRING_START (1<<xstring_start)
#define UNICODE_ESCAPE (1<<xunicode_escape)
#define XDIGIT (1<<xdigit)
#define XDOT (1<<xdot)
#define XMINUS (1<<xminus)

char * input_expectation[n_expectations] = {
    "whitespace",
    "start of a value, 0-9, '-', '\"', 't', 'n', 'f', '[', or '{'",
    "comma ','",
    "value separator ':'",
    "end of object '}'",
    "end of array ']'",
    "hexadecimal character, 0-9, a-f or A-F",
    "start of string, '\"'",
    "unicode escape \\uXXXX",
    "digit 0-9",
    "dot .",
    "minus '-'",
};

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

    /* Where the beginning of the series of unfortunate events was. */

    char * bad_beginning;

    /* The bad type itself. */

    json_type_t bad_type;

    /* What we were expecting to see when the error occurred. */

    int expected;

    /* The byte which caused the parser to fail. */

    char * bad_byte;

    /* The type of error encountered. */

    json_error_t error;

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

#define ERRORMSGBUFFERSIZE 0x1000

/* Error fallthrough. This takes the error and sends it to "croak". */

#define NOBYTE -1
#define HEXFORMAT "0x%02x"
#define XLENGTH strlen (HEXFORMAT)

/* Assert failure handler. Coming here means there is a bug in the
   code rather than in the JSON input. We still send it to Perl via
   "croak". */

static INLINE void
failbug (char * file, int line, parser_t * parser, const char * format, ...)
{
    char buffer[ERRORMSGBUFFERSIZE];
    va_list a;
    va_start (a, format);
    vsnprintf (buffer, ERRORMSGBUFFERSIZE, format, a);
    va_end (a);
    croak ("JSON::Parse: %s:%d: Internal error at line %d: %s",
	   file, line, parser->line, buffer);
}

/* This is a test for whether the string has ended, which we use when
   we catch a zero byte in an unexpected part of the input. Here we
   use ">" rather than ">=" because "parser->end" is incremented by
   one after each access. */

#define STRINGEND (parser->end > parser->last_byte)

static INLINE void
failbadinput (parser_t * parser)
{
    char buffer[ERRORMSGBUFFERSIZE];
    char formatbuffer[ERRORMSGBUFFERSIZE];
    const char * format;
    int string_end;
    int i;
    int l;
    if (
	(
	 parser->error == json_error_unexpected_character
	 ||
	 parser->error == json_error_illegal_byte
	 )
	&&
	STRINGEND) {
	parser->error = json_error_unexpected_end_of_input;
	parser->bad_byte = 0;
    }
    if (parser->error != json_error_invalid &&
	parser->error < json_error_overflow) {
	format = json_errors[parser->error];
    }
    else {
	failbug (__FILE__, __LINE__, parser,
		 "Bad value for parser->error: %d\n", parser->error);
    }
    l = strlen (format);
    if (l > ERRORMSGBUFFERSIZE - XLENGTH) {
	l = ERRORMSGBUFFERSIZE - XLENGTH;
    }
    for (i = 0; i < l; i++) {
	formatbuffer[i] = format[i];
    }
    formatbuffer[l] = '\0';
    /* If the error is unexpected character or illegal byte, and the
       actual unexpected character is the end of the string character
       \0, change to unexpected end of input error. */
    if (parser->bad_byte) {
	if (! isprint (* (parser->bad_byte))) {
	    int percent;
	    int j;
	    percent = 0;
	    for (i = 0, j = 0; i < l; i++, j++) {
		if (percent) {
		    if (format[i] == 'c') {
			int k;
			j -= 2;
			for (k = 0; k < XLENGTH; k++, j++) {
			    formatbuffer[j] = HEXFORMAT[k];
			}
			/* Skip the 'c' and trailing '. */
			i += 2;
		    }
		}
		if (format[i] == '%') {
		    percent = 1;
		}
		else {
		    percent = 0;
		}
		formatbuffer[j] = format[i];
	    }
	}
	string_end = snprintf (buffer, ERRORMSGBUFFERSIZE, formatbuffer,
			       * parser->bad_byte);
    }
    else {
	string_end = snprintf (buffer, ERRORMSGBUFFERSIZE, formatbuffer);
    }
    if (parser->bad_type) {
	if (parser->bad_type < json_overflow) {
	    string_end += snprintf (buffer + string_end,
				    ERRORMSGBUFFERSIZE - string_end,
				    " parsing %s",
				    type_names[parser->bad_type]);
	    if (parser->bad_beginning) {
		string_end += snprintf (buffer + string_end,
					ERRORMSGBUFFERSIZE - string_end,
					" starting from byte %d",
					parser->bad_beginning
					- parser->input + 1);
	    }
	    if (parser->expected) {
		int i;
		int joined;
		string_end += snprintf (buffer + string_end,
					ERRORMSGBUFFERSIZE
					- string_end,
					" expecting ");
		joined = 0;
		for (i = 0; i < n_expectations; i++) {
		    if (parser->expected & (1<<i)) {
			if (joined) {
			    string_end += snprintf (buffer + string_end,
						    ERRORMSGBUFFERSIZE
						    - string_end,
						    " or ");
			}
			string_end += snprintf (buffer + string_end,
						ERRORMSGBUFFERSIZE - string_end,
						"%s", input_expectation[i]);
			joined = 1;
		    }
		}
	    }
	}
    }
    if (parser->length > 0) {
	if (parser->end - parser->input > parser->length) {
	    croak ("JSON error at line %d: %s", parser->line,
		   buffer);
	}
	else if (parser->bad_byte) {
	    croak ("JSON error at line %d, byte %d/%d: %s",
		   parser->line,
		   parser->bad_byte - parser->input + 1,
		   parser->length, buffer);
	}
	else {
	    croak ("JSON error at line %d: %s",
		   parser->line, buffer);
	}
    }
    else {
	croak ("JSON error: %s", buffer);
    }
}

/* This is for failures not due to errors in the input or to bugs but
   to exhaustion of resources, i.e. out of memory. */

static INLINE void failresources (parser_t * parser, const char * format, ...)
{
    char buffer[ERRORMSGBUFFERSIZE];
    va_list a;
    va_start (a, format);
    vsnprintf (buffer, ERRORMSGBUFFERSIZE, format, a);
    va_end (a);
    croak ("Parsing failed at line %d, byte %d/%d: %s", parser->line,
	   parser->end - parser->input,
	   parser->length, buffer);
}

#undef ERRORMSGBUFFERSIZE

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
	    failresources (parser, "out of memory");
	}
    }
}

/* The following are used in parsing \u escapes only. */

#define LHEXBYTE						     \
      'a': case 'b': case 'c': case 'd': case 'e': case 'f'

#define UHEXBYTE						     \
      'A': case 'B': case 'C': case 'D': case 'E': case 'F' 

#define UNIFAIL(err)						\
    parser->bad_type = json_unicode_escape;			\
    parser->expected = HEXADECIMAL_CHARACTER;			\
    parser->error = json_error_ ## err;				\
    failbadinput (parser)

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
		UNIFAIL (unexpected_end_of_input);
	    }

	    /* Fallthrough */

	default:
	    parser->bad_byte = p + k;
	    UNIFAIL (non_hexadecimal_character);
	}
    }
    return unicode;
}

#undef LHEXBYTE
#undef UHEXBYTE

#define STRINGFAIL(err)				\
    parser->error = json_error_ ## err;		\
    parser->bad_type = json_string;		\
    failbadinput (parser)

static INLINE char *
do_unicode_escape (parser_t * parser, char * p, unsigned char ** b_ptr)
{
    int unicode;
    unsigned int plus;
    unicode = parse_hex_bytes (parser, p);
    p += 4;
    plus = ucs2_to_utf8 (unicode, *b_ptr);
    if (plus == UNICODE_BAD_INPUT) {
	failbadinput (parser);
    }
    else if (plus == UNICODE_SURROGATE_PAIR) {
	int unicode2;
	int plus2;
	if (*p++ == '\\' && *p++ == 'u') {
	    unicode2 = parse_hex_bytes (parser, p);
	    p += 4;
	    plus2 = surrogate_to_utf8 (unicode, unicode2, * b_ptr);
	    if (plus2 <= 0) {
		parser->bad_byte = 0;
		parser->bad_beginning = p - 4;
		UNIFAIL (surrogate_pair_unreadable);
	    }
	    * b_ptr += plus2;
	    goto end;
	}
	else {
	    parser->bad_byte = p - 1;
	    parser->expected = UNICODE_ESCAPE;
	    STRINGFAIL (second_half_of_surrogate_pair_missing);
	}
    }
    else if (plus <= 0) {
	failbug (__FILE__, __LINE__, parser, 
		 "unhandled error code %d while decoding unicode escape",
		 plus);
    }
    * b_ptr += plus;
 end:
    if (unicode >= 0x80 && ! parser->unicode) {
	parser->force_unicode = 1;
    }
    return p;
}

/* Handle backslash escapes. We can't use the NEXTBYTE macro here for
   the reasons outlined below. */

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
    case BADBYTES:					\
	parser->bad_beginning = p - 2;			\
	parser->bad_byte = p - 1;			\
	STRINGFAIL (unexpected_character);		\
	break;						\
    default:						\
	parser->bad_beginning = p - 2;			\
	parser->bad_byte = p - 1;			\
	STRINGFAIL (unknown_escape);			\
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
	    parser->bad_beginning = key->start - 1;
	    STRINGFAIL (unexpected_end_of_input);
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

#define ILLEGALBYTE							\
    parser->bad_beginning = start;					\
    parser->bad_byte = parser->end - 1;					\
    STRINGFAIL (illegal_byte)


/* Resolve the string pointed to by "parser->end" into
   "parser->buffer". The return value is the length of the
   string. This is only called if the string has \ escapes in it. */

static INLINE int
get_string (parser_t * parser)
{
    unsigned char * b;
    unsigned char c;
    char * start;

    start = parser->end;

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
	if (STRINGEND) {
	    STRINGFAIL (unexpected_end_of_input);
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


