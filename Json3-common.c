#include "utf8-bytes.c"

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
   not by Perl. 

   "Insignificant whitespace is allowed before or after any of the six
   structural characters.

      ws = *(
                %x20 /              ; Space
                %x09 /              ; Horizontal tab
                %x0A /              ; Line feed or New line
                %x0D                ; Carriage return
            )"
    
   From JSON RFC.
*/

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

/* Match digits from 1-9. This is handled differently because JSON
   disallows leading zeros in numbers. */

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

#define UHEX  'A': case 'B': case 'C': case 'D': case 'E': case 'F'
#define LHEX  'a': case 'b': case 'c': case 'd': case 'e': case 'f'

/* A "string_t" is a pointer into the input, which lives in
   "parser->input". The "string_t" structure is used for copying
   strings when the string does not contain any escapes. When a string
   contains escapes, it is copied into "parser->buffer". */

typedef struct string {

    unsigned char * start;
    unsigned int length;

    /* The "contains_escapes" flag is set if there are backslash escapes in
       the string like "\r", so that it needs to be cleaned up before
       using it. That means we use "parser->buffer". This is to speed
       things up, by not doing the cleanup when it isn't necessary. */

    unsigned contains_escapes : 1;
}
string_t;

typedef enum {
    json_invalid,
    json_initial_state,
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
    "initial state",
    "string",
    "number",
    "literal",
    "object",
    "array",
    "unicode escape"
};

#define MAXBYTE 0x100

#include "errors.c"

/* Anything which could be the start of a value. */

#define VALUE_START (XARRAYOBJECTSTART|XSTRING_START|XDIGIT|XMINUS|XLITERAL)

/* The maximum value of bytes to check for. Once UTF-8 is included in
   the module, this will change to 0x100. */

typedef struct parser {

    /* The length of "input". */

    unsigned int length;

    /* The input. */

    unsigned char * input;

    /* The end-point of the parsing. This increments through
       "input". */

    unsigned char * end;

    /* The last byte of "input", "parser->input +
       parser->length". This is used to detect overflows. */

    unsigned char * last_byte;

    /* Allocated size of "buffer". */

    int buffer_size;

    /* Buffer to stick strings into temporarily. */

    unsigned char * buffer;

    /* Line number. */

    int line;

    /* Where the beginning of the series of unfortunate events
       was. For example if we are parsing an array, this points to the
       [ at the start of the array, or if we are parsing a string,
       this points to the byte after " at the start of the string. */

    unsigned char * bad_beginning;

    /* The bad type itself. */

    json_type_t bad_type;

    /* What we were expecting to see when the error occurred. */

    int expected;

    /* The byte which caused the parser to fail. */

    unsigned char * bad_byte;

    /* The type of error encountered. */

    json_error_t error;

    /* If we were parsing a literal and found a bad character, what
       were we expecting? */
    
    unsigned char literal_char;

    /* The end expected. */

    int end_expected;

#ifdef TESTRANDOM

    /* Return point for longjmp. */

    jmp_buf biscuit;

    int valid_bytes[MAXBYTE];

    char * last_error;

#endif

    /* Unicode? */

    unsigned int unicode : 1;

    /* Force unicode. This happens when we hit "\uxyzy". */

    unsigned int force_unicode : 1;

#ifdef TESTRANDOM

    /* This is true if we are testing with random bytes. */

    unsigned randomtest : 1;

#endif /* def TESTRANDOM */
}
parser_t;

#ifdef __GNUC__
#define INLINE inline
#else
#define INLINE
#endif /* def __GNUC__ */

/* The size of the buffer for printing errors. */

#define ERRORMSGBUFFERSIZE 0x1000

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
   one after each access. See the NEXTBYTE macro. */

#define STRINGEND (parser->end > parser->last_byte)

static INLINE void
failbadinput (parser_t * parser)
{
    char buffer[ERRORMSGBUFFERSIZE];
    const char * format;
    int string_end;
    int i;
    int l;

    /* If the error is "unexpected character", and we are at the end
       of the input, change to "unexpected end of input". This is
       probably triggered by reading a byte with value '\0', but we
       don't check the value of "* parser->bad_byte" in the following
       "if" statement, since it's an error to go past the expected end
       of the string regardless of whether the byte is '\0'. */

    if (parser->error == json_error_unexpected_character &&
	STRINGEND) {
	parser->error = json_error_unexpected_end_of_input;
	/* We don't care about what byte it was, we went past the end
	   of the string, which is already a failure. */
	parser->bad_byte = 0;
	/* It trips an assertion if "parser->expected" is set for
	   anything other than an "unexpected character" error. */
	parser->expected = 0;
    }
    /* Array bounds check for error message. */
    if (parser->error != json_error_invalid &&
	parser->error < json_error_overflow) {
	format = json_errors[parser->error];
    }
    else {
	failbug (__FILE__, __LINE__, parser,
		 "Bad value for parser->error: %d\n", parser->error);
    }
    l = strlen (format);
    if (l > ERRORMSGBUFFERSIZE - 1) {
	/* Probably should report a bug here rather than coping with
	   it. */
	l = ERRORMSGBUFFERSIZE - 1;
    }
    for (i = 0; i < l; i++) {
	buffer[i] = format[i];
    }
    buffer[l] = '\0';
    string_end = l;

    /* Repeated arguments to snprintf. */

#define SNARGS buffer + string_end, ERRORMSGBUFFERSIZE - string_end

    /* If we got an unexpected character somewhere, append the exact
       value of the character to the error message. */

    if (parser->error == json_error_unexpected_character) {

	/* This contains the unexpected character itself, from the
	   "parser->bad_byte" pointer. */

	unsigned char bb;

	/* Make sure that we were told where the unexpected character
	   was. Unlocated unexpected characters are a bug. */

	if (! parser->bad_byte) {
	    failbug (__FILE__, __LINE__, parser,
		     "unexpected character error but "
		     "parser->bad_byte is invalid");
	}

	bb = * parser->bad_byte;

	/* We have to check what kind of character. For example
	   printing '\0' with %c will just give a message which
	   suddenly ends when printed to the terminal, and other
	   control characters will be invisible. So display the
	   character in a different way depending on whether it's
	   printable or not. */

	if (isprint (bb)) {
	    /* Printable character, print the character itself. */
	    string_end += snprintf (SNARGS, " '%c'", bb);
	}
	else {
	    /* Unprintable character, print its hexadecimal value. */
	    string_end += snprintf (SNARGS, " 0x%02x", bb);
	}
    }
    /* "parser->bad_type" contains what was being parsed when the
       error occurred. This should never be undefined. */
    if (parser->bad_type <= json_invalid ||
	parser->bad_type >= json_overflow) {
	failbug (__FILE__, __LINE__, parser,
		 "parsing type set to invalid value %d in error message",
		 parser->bad_type);
    }
    string_end += snprintf (SNARGS, " parsing %s",
			    type_names[parser->bad_type]);
    if (parser->bad_beginning) {
	string_end += snprintf (SNARGS, " starting from byte %d",
				parser->bad_beginning - parser->input + 1);
    }

    /* "parser->expected" is set for the "unexpected character" error
       and it tells the user what kind of input was expected. It
       contains various flags or'd together, so this goes through each
       possible flag and prints a message for it. */

    if (parser->expected) {
	if (parser->error == json_error_unexpected_character) {
	    int i;
	    int joined;
	    unsigned char bb;
	    bb = * parser->bad_byte;

	    string_end += snprintf (SNARGS, ": expecting ");
	    joined = 0;
#ifdef TESTRANDOM
	    for (i = 0; i < MAXBYTE; i++) {
		parser->valid_bytes[i] = 0;
	    }
#endif /* def TESTRANDOM */
	    if (parser->expected & XIN_LITERAL) {
		if (! parser->literal_char) {
		    failbug (__FILE__, __LINE__, parser,
			     "expected literal character unset");
		}
#ifdef TESTRANDOM
		parser->valid_bytes[parser->literal_char] = 1;
#endif /* def TESTRANDOM */
	    }
	    for (i = 0; i < n_expectations; i++) {
		int X;
		X = 1<<i;
		if (X & XIN_LITERAL) {
		    continue;
		}
		if (parser->expected & X) {
#ifdef TESTRANDOM
		    int j;
		    for (j = 0; j < MAXBYTE; j++) {
#if 0
			/* This is to check we have meaningful stuff
			   in the valid/invalid table. */
			if (parser->randomtest) {
			    printf ("%d", allowed[i][j]);
			}
#endif /* 0 */
			parser->valid_bytes[j] |= allowed[i][j];
		    }
#if 0
		    if (parser->randomtest) {
			printf ("\n");
		    }
#endif /* 0 */
#endif /* def TESTRANDOM */

		    /* Check that this really is disallowed. */
		    
		    if (i != xin_literal) {
			if (allowed[i][bb]) {
			    failbug (__FILE__, __LINE__, parser,
				     "mismatch: got %X but it's allowed by %s",
				     bb, input_expectation[i]);
			}
		    }
		    if (joined) {
			string_end += snprintf (SNARGS, " or ");
		    }
		    string_end += snprintf (SNARGS, "%s", input_expectation[i]);
		    joined = 1;
		}
	    }
	}
	else {
	    failbug (__FILE__, __LINE__, parser,
		     "'expected' is set but error %s != unexp. char",
		     json_errors[parser->error]);
	}
    }
    else if (parser->error == json_error_unexpected_character) {
	failbug (__FILE__, __LINE__, parser,
		 "unexpected character error for 0X%02X at byte %d "
		 "with no expected value set", * parser->bad_byte,
		 parser->bad_byte - parser->input);
    }

#undef SNARGS

#ifdef TESTRANDOM

    /* Go back to where we came from. */

    if (parser->randomtest) {
	parser->last_error = buffer;
	longjmp (parser->biscuit, 1);
    }

#endif /* def TESTRANDOM */

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
	    Renew (parser->buffer, parser->buffer_size, unsigned char);
	}
	else {
	    Newx (parser->buffer, parser->buffer_size, unsigned char);
	}
	if (! parser->buffer) {
	    failresources (parser, "out of memory");
	}
    }
}

#define UNIFAIL(err)						\
    parser->bad_type = json_unicode_escape;			\
    parser->error = json_error_ ## err;				\
    failbadinput (parser)

/* Parse the hex bit of a \uXYZA escape. */

static INLINE int
parse_hex_bytes (parser_t * parser, unsigned char * p)
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

	case UHEX:
	    unicode = unicode * 16 + c - 'A' + 10;
	    break;

	case LHEX:
	    unicode = unicode * 16 + c - 'a' + 10;
	    break;

	case '\0':
	    if (p + k - parser->input >= parser->length) {
		UNIFAIL (unexpected_end_of_input);
	    }

	    /* Fallthrough */

	default:
	    parser->bad_byte = p + k;
	    parser->expected = XHEXADECIMAL_CHARACTER;
	    UNIFAIL (unexpected_character);
	}
    }
    return unicode;
}

/* STRINGFAIL applies for any kind of failure within a string, not
   just unexpected character errors. */

#define STRINGFAIL(err)				\
    parser->error = json_error_ ## err;		\
    parser->bad_type = json_string;		\
    failbadinput (parser)

static INLINE unsigned char *
do_unicode_escape (parser_t * parser, unsigned char * p, unsigned char ** b_ptr)
{
    int unicode;
    unsigned int plus;
    unsigned char * start;
    start = p;
    unicode = parse_hex_bytes (parser, p);
    p += 4;
    plus = ucs2_to_utf8 (unicode, *b_ptr);
    if (plus == UNICODE_BAD_INPUT) {
	parser->bad_beginning = start;
	UNIFAIL (bad_unicode_input);
    }
    else if (plus == UNICODE_SURROGATE_PAIR) {
	int unicode2;
	int plus2;
	if (*p++ == '\\' && *p++ == 'u') {
	    unicode2 = parse_hex_bytes (parser, p);
	    p += 4;
	    plus2 = surrogate_to_utf8 (unicode, unicode2, * b_ptr);
	    if (plus2 <= 0) {
		if (plus2 == UNICODE_NOT_SURROGATE_PAIR) {
		    parser->bad_byte = 0;
		    parser->bad_beginning = p - 4;
		    UNIFAIL (not_surrogate_pair);
		}
		else {
		    failbug (__FILE__, __LINE__, parser,
			     "unhandled error %d from surrogate_to_utf8",
			     plus2);
		}
	    }
	    * b_ptr += plus2;
	    goto end;
	}
	else {
	    parser->bad_byte = p - 1;
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
	/* Force the UTF-8 flag on for this string. */
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
							\
    default:						\
	parser->bad_beginning = p - 2;			\
	parser->bad_byte = p - 1;			\
        parser->expected = XESCAPE;			\
	STRINGFAIL (unexpected_character);		\
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

    unsigned char * p;

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

#define NEXTBYTE (c = *parser->end++)

/* Get an object key value and put it into "key". Check for
   escapes. */

static INLINE void
get_key_string (parser_t * parser, string_t * key)
{
    unsigned char c;
    int i;

    key->start = parser->end;
    key->contains_escapes = 0;

 key_string_next:

    switch (NEXTBYTE) {

    case '"':
	/* Go on eating bytes until we find a ". */

	break;

    case '\\':
	/* Mark this string as containing escapes. */
	key->contains_escapes = 1;

	switch (NEXTBYTE) {

	case '\\':
	case '/': 
	case '"': 
	case 'b':
	case 'f': 
	case 'n': 
	case 'r': 
	case 't': 
	    /* Eat another byte. */
	    goto key_string_next;

	case 'u': 

	    /* i counts the bytes, from 0 to 3. */
	    i = 0;
	unitunes:
	    switch (NEXTBYTE) {
	    case DIGIT:
	    case UHEX:
	    case LHEX:
		i++;
		if (i >= strlen ("ABCD")) {
		    goto key_string_next;
		}
		else {
		    goto unitunes;
		}
	    default:
		parser->bad_beginning = parser->end - 1 - i;
		parser->expected = XHEXADECIMAL_CHARACTER;
		parser->bad_byte = parser->end - 1;
		UNIFAIL (unexpected_character);
	    }

	default:
	    parser->bad_beginning = key->start - 1;
	    parser->expected = XESCAPE;
	    parser->bad_byte = parser->end - 1;
	    STRINGFAIL (unexpected_character);
	}

    case BADBYTES:

	parser->bad_beginning = key->start - 1;
	parser->expected = XSTRINGCHAR;
	parser->bad_byte = parser->end - 1;
	STRINGFAIL (unexpected_character);

    default:

	/* Do nothing. */
#define ADDBYTE 
#define string_start key_string_next
#include "utf8-byte-one.c"
    }
    key->length = parser->end - key->start - 1;
    return;

#include "utf8-next-byte.c"
#undef string_start
#undef ADDBYTE
}

#define ILLEGALBYTE							\
    parser->bad_beginning = start;					\
    parser->bad_byte = parser->end - 1;					\
    parser->expected = XSTRINGCHAR;					\
    STRINGFAIL (unexpected_character)


/* Resolve the string pointed to by "parser->end" into
   "parser->buffer". The return value is the length of the
   string. This is only called if the string has \ escapes in it. */

static INLINE int
get_string (parser_t * parser)
{
    unsigned char * b;
    unsigned char c;
    unsigned char * start;

    start = parser->end;

    if (! parser->buffer) {
	expand_buffer (parser, 0x1000);
    }
    b = parser->buffer;

 string_start:

    if (b - parser->buffer >= parser->buffer_size - 0x100) {
	/* Save our offset in parser->buffer, because "realloc" is
	   called by "expand_buffer", and "b" may no longer point
	   to a meaningful location. */
	int size = b - parser->buffer;
	expand_buffer (parser, 2 * parser->buffer_size);
	b = parser->buffer + size;
    }
    switch (NEXTBYTE) {

    case '"':
	goto string_end;
	break;

    case '\\':
	HANDLE_ESCAPES(parser->end);
	goto string_start;

    case BADBYTES:
	ILLEGALBYTE;

#define ADDBYTE (* b++ = c)
#include "utf8-byte-one.c"

    default:
	ILLEGALBYTE;
    }

    if (STRINGEND) {
	STRINGFAIL (unexpected_end_of_input);
    }

    goto string_end;

#include "utf8-next-byte.c"
#undef ADDBYTE

 string_end:
    return b - parser->buffer;
}

static void
parser_free (parser_t * parser)
{
    if (parser->buffer) {
	Safefree (parser->buffer);
    }
}


