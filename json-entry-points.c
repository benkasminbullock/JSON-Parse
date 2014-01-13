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

static void
print_tokens (json_token_t * t)
{
    printf ("Start: %d End: %d: Type: %s\n", t->start, t->end,
	    token_names[t->type]);
    if (t->child) {
	printf ("Children:\n");
	print_tokens (t->child);
    }
    if (t->next) {
	printf ("Next:\n");
	print_tokens (t->next);
    }
}

#ifndef NOPERL

static json_token_t *
c_tokenize (parser_t * parser)
{
    /* The currently-parsed character. */	
						
    char c;					
    json_token_t * r;

    SETUPPARSER;

 tokenize_start:

    switch (NEXTBYTE) {

    case '{':
	r = tokenize_object (parser);
	break;

    case '[':
	r = tokenize_array (parser);
	break;

    case WHITESPACE:
	goto tokenize_start;

    default:
	BADCHAR;
    }

    check_end (parser);
    /*
    printf ("TOKENS:\n");
    print_tokens (r);
    */
    return r;
}

static void
tokenize_free (json_token_t * token)
{
    json_token_t * next;
    //    printf ("Freeing tokens.\n");
    next = token->child;
    if (next) {
	tokenize_free (next);
    }
    next = token->next;
    if (next) {
	tokenize_free (next);
    }
    Safefree (token);
}

/* This is the entry point for validation. */

static void
validate (SV * json)
{
    ENTRYDECL;

    GETSTRING;

    c_validate (& parser_o);
}

static json_token_t *
tokenize (SV * json)
{
    ENTRYDECL;

    GETSTRING;

    return c_tokenize (& parser_o);
}

#endif /* ndef NOPERL */

