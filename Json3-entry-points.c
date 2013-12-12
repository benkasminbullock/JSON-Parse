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
	failburger (parser,
		    "Stray character '%c' after end of object/array", c);
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

#define SETUPPARSER(fail)						\
    parser_o.end = parser_o.input = SvPV (json, parser_o.length);	\
    if (parser_o.length == 0) {						\
	fail;								\
    }									\
    parser_o.line = 1;							\
    parser_o.last_byte = parser_o.input + parser_o.length;		\
    parser_o.unicode = SvUTF8 (json) ? 1 : 0;

/* Error to throw if there is a character other than whitespace, "["
   or "{" at the start of the JSON. */

#define BADCHAR \
    failburger (& parser_o, "Bad character '%c' in initial state", c)


/* This is the entry point for parsing. */

static SV *
parse (SV * json)
{
    ENTRYDECL;

    /* The returned object. */

    SV * r;

    SETUPPARSER (return & PL_sv_undef);

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

    /* If the string is empty, throw an exception. */

    SETUPPARSER (failburger (& parser_o, "Empty input"));

 validate_start:

    switch (NEXTBYTE) {

    case '{':
	valid_object (& parser_o);
	break;

    case '[':
	valid_array (& parser_o);
	break;

    case '\0':
	failburger (& parser_o, "Empty input");

    case WHITESPACE:
	goto validate_start;

    default:
	BADCHAR;
    }

    check_end (parser);
}
