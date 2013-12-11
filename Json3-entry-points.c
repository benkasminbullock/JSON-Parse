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

/* This is the entry point for parsing. */

static SV *
parse (SV * json)
{
    /* The currently-parsed character. */

    char c;

    /* Our collection of bits and pieces. */

    parser_t parser_o = {0};
    parser_t * parser = & parser_o;

    /* The returned object. */

    SV * r;

    /* Set up the object. */

    parser_o.end = parser_o.input = SvPV (json, parser_o.length);

    if (parser_o.length == 0) {
	return & PL_sv_undef;
    }
    parser_o.line = 1;
    parser_o.last_byte = parser_o.input + parser_o.length;
    parser_o.unicode = SvUTF8 (json) ? 1 : 0;

 parse_start:

    switch (c = *parser_o.end++) {

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
	break;

    default:
	failburger (& parser_o, "Bad character '%c' in initial state", c);
    }

    check_end (parser);

    return r;
}

/* This is the entry point for validation. */

static int
validate (SV * json)
{
    /* The currently-parsed character. */

    char c;

    /* Our collection of bits and pieces. */

    parser_t parser_o = {0};
    parser_t * parser = & parser_o;

    /* Set up the object. */

    parser_o.end = parser_o.input = SvPV (json, parser_o.length);

    /* If the string is empty, throw an exception. */

    if (parser_o.length == 0) {
	failburger (& parser_o, "Empty input");
    }

    parser_o.line = 1;
    parser_o.last_byte = parser_o.input + parser_o.length;
    parser_o.unicode = SvUTF8 (json) ? 1 : 0;

 validate_start:

    switch (c = *parser_o.end++) {

    case '{':
	valid_object (& parser_o);
	break;

    case '[':
	valid_array (& parser_o);
	break;

    case WHITESPACE:
	goto validate_start;

    case '\0':
	failburger (& parser_o, "Empty input");

    default:
	failburger (& parser_o, "Bad character '%c' in initial state", c);
    }

    check_end (parser);

    return 1;
}
