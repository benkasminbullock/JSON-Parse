/* Check for stray non-whitespace after the end and free memory. */

static void check_end (parser_t * parser)
{
    int c;
 end:
    switch (NEXTBYTE) {

    case WHITESPACE:
	goto end;

    case '\0':
	return;

    default:
	parser->bad_type = json_initial;
	parser->bad_byte = parser->end - 1;
	parser->expected = XWHITESPACE;
	parser->error = json_error_unexpected_character;
	failbadinput (parser);
    }
    parser_free (parser);
}

#define ENTRYDECL				\
    /* The currently-parsed character. */	\
						\
    char c;					\
						\
    /* Our collection of bits and pieces. */	\
						\
    parser_t parser_o = {0};			\
    parser_t * parser = & parser_o

/* Set up the parser. */

#define GETSTRING							\
    parser_o.end = parser_o.input = SvPV (json, parser_o.length)

#define SETUPPARSER							\
    parser_o.line = 1;							\
    parser_o.last_byte = parser_o.input + parser_o.length;		\
    parser_o.unicode = SvUTF8 (json) ? 1 : 0

/* Error to throw if there is a character other than whitespace, "["
   or "{" at the start of the JSON. */

#define BADCHAR								\
    parser->bad_byte = parser->end - 1;					\
    parser->bad_type = json_initial;					\
    parser->expected = XARRAYOBJECTSTART;				\
    parser->error = json_error_unexpected_character;			\
    failbadinput (parser)

/* This is the entry point for parsing. */

static SV *
parse (SV * json)
{
    ENTRYDECL;

    /* The returned object. */

    SV * r;

    GETSTRING;

    if (parser_o.length == 0) {		
	return & PL_sv_undef;
    }						

    SETUPPARSER;

 parse_start:

    switch (NEXTBYTE) {

    case '{':
	r = object (& parser_o);
	break;

    case '[':
	r = array (& parser_o);
	break;

    case WHITESPACE:
	goto parse_start;

    case '\0':

	/* We have an empty string. */

	r = & PL_sv_undef;

	/* So there is no point checking that the string doesn't
	   contain junk characters after its end. */

	goto noendcheck;

    default:
	BADCHAR;
    }

    check_end (parser);

 noendcheck:

    return r;
}

/* This is the entry point for validation. */

static void
validate (SV * json)
{
    ENTRYDECL;

    GETSTRING;

    /* If the string is empty, throw an exception. */
    
    if (parser_o.length == 0) {		
	parser->error = json_error_empty_input;
	failbadinput (& parser_o);
    }						

    SETUPPARSER;

 validate_start:

    switch (NEXTBYTE) {

    case '{':
	valid_object (& parser_o);
	break;

    case '[':
	valid_array (& parser_o);
	break;

    case '\0':
	parser->error = json_error_empty_input;
	failbadinput (& parser_o);

    case WHITESPACE:
	goto validate_start;

    default:
	BADCHAR;
    }

    check_end (parser);
}
