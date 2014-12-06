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
	parser->bad_type = json_initial_state;
	parser->bad_byte = parser->end - 1;
	parser->expected = XWHITESPACE;
	parser->error = json_error_unexpected_character;
	failbadinput (parser);
    }
    parser_free (parser);
}

#define ENTRYDECL				\
    /* Our collection of bits and pieces. */	\
						\
    parser_t parser_o = {0};			\
    parser_t * parser = & parser_o

#ifndef NOPERL

/* Set up the parser. */

#define GETSTRING							\
    parser_o.end = parser_o.input =					\
	(unsigned char *) SvPV (json, parser_o.length);			\
    parser->unicode = SvUTF8 (json) ? 1 : 0

#endif /* ndef NOPERL */

#define SETUPPARSER							\
    parser->line = 1;							\
    parser->last_byte = parser->input + parser->length

/* Error to throw if there is a character other than whitespace, "["
   or "{" at the start of the JSON. */

#define BADCHAR								\
    parser->bad_byte = parser->end - 1;					\
    parser->bad_type = json_initial_state;				\
    parser->expected = XARRAYOBJECTSTART | XWHITESPACE;			\
    parser->error = json_error_unexpected_character;			\
    failbadinput (parser)

#ifndef NOPERL

/* This is the entry point for parsing. */

static SV *
parse (SV * json)
{
    /* The currently-parsed character. */	
						
    char c;					
						
    ENTRYDECL;

    /* The returned object. */

    SV * r = & PL_sv_undef;

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

#endif /* ndef NOPERL */

/* Validation without Perl structures. */

static void
c_validate (parser_t * parser)
{
    /* The currently-parsed character. */	
						
    char c;					

    /* If the string is empty, throw an exception. */

    if (parser->length == 0) {
	parser->bad_type = json_initial_state;
	parser->error = json_error_empty_input;
	failbadinput (parser);
    }

    SETUPPARSER;

 validate_start:

    switch (NEXTBYTE) {

    case '{':
	valid_object (parser);
	break;

    case '[':
	valid_array (parser);
	break;

    case WHITESPACE:
	goto validate_start;

    default:
	BADCHAR;
    }

    check_end (parser);
}

#ifndef NOPERL

/* This is the entry point for validation. */

static void
validate (SV * json)
{
    ENTRYDECL;

    GETSTRING;

    c_validate (& parser_o);
}

#endif /* ndef NOPERL */

