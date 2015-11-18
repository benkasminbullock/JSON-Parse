/* Check for stray non-whitespace after the end and free memory. */

static void check_end (json_parse_t * parser)
{
    int c;
 end:
    switch (NEXTBYTE) {

    case WHITESPACE:
	goto end;

    case '\0':
	parser_free (parser);
	return;

    default:
	parser->bad_type = json_initial_state;
	parser->bad_byte = parser->end - 1;
	parser->expected = XWHITESPACE;
	parser->error = json_error_unexpected_character;
	failbadinput (parser);
    }
}

#define ENTRYDECL				\
    /* Our collection of bits and pieces. */	\
						\
    json_parse_t parser_o = {0};			\
    json_parse_t * parser = & parser_o

#ifndef NOPERL

/* Set up the parser. */

#define GETSTRING							\
    {									\
	STRLEN length;							\
	parser->end = parser->input =					\
	    (unsigned char *) SvPV (json, length);			\
	parser->length = (unsigned int) length;				\
	parser->unicode = SvUTF8 (json) ? 1 : 0;			\
    }

#endif /* ndef NOPERL */

#define SETUPPARSER							\
    parser->line = 1;							\
    parser->last_byte = parser->input + parser->length

/* Error to throw if there is a character other than whitespace, "["
   or "{" at the start of the JSON. */

#define BADCHAR								\
    parser->bad_byte = parser->end - 1;					\
    parser->bad_type = json_initial_state;				\
    parser->expected = XARRAYOBJECTSTART | VALUE_START | XWHITESPACE;	\
    parser->error = json_error_unexpected_character;			\
    failbadinput (parser)

#ifndef NOPERL

static SV *
json_parse_run (json_parse_t * parser, SV * json)
{
    /* The currently-parsed character. */	
						
    char c;					
						
    /* The returned object. */

    SV * r = & PL_sv_undef;

    GETSTRING;

    if (parser->length == 0) {
	return & PL_sv_undef;
    }

    SETUPPARSER;

 parse_start:

    switch (NEXTBYTE) {

    case '{':
	r = object (parser);
	break;

    case '[':
	r = array (parser);
	break;

    case '-':
    case '0':
    case DIGIT19:
	parser->top_level_value = 1;
	r = number (parser);
	break;

    case '"':
	parser->top_level_value = 1;
	r = string (parser);
	break;

    case 't':
	parser->top_level_value = 1;
	r = literal_true (parser);
	break;

    case 'f':
	parser->top_level_value = 1;
	r = literal_false (parser);
	break;

    case 'n':
	parser->top_level_value = 1;
	r = literal_null (parser);
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

/* This is the entry point for non-object parsing. */

SV *
parse (SV * json)
{
    /* Make our own parser object on the stack. */
    ENTRYDECL;
    /* Run it. */
    return json_parse_run (& parser_o, json);
}


/* This is the entry point for "safe" non-object parsing. */

SV *
parse_safe (SV * json)
{
    /* Make our own parser object on the stack. */
    ENTRYDECL;
    parser_o.detect_collisions = 1;
    parser_o.copy_literals = 1;
    parser_o.warn_only = 1;
    /* Run it. */
    return json_parse_run (& parser_o, json);
}


#endif /* ndef NOPERL */

/* Validation without Perl structures. */

static void
c_validate (json_parse_t * parser)
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

    case '-':
    case '0':
    case DIGIT19:
	parser->top_level_value = 1;
	valid_number (parser);
	break;

    case '"':
	parser->top_level_value = 1;
	valid_string (parser);
	break;

    case 't':
	parser->top_level_value = 1;
	valid_literal_true (parser);
	break;

    case 'f':
	parser->top_level_value = 1;
	valid_literal_false (parser);
	break;

    case 'n':
	parser->top_level_value = 1;
	valid_literal_null (parser);
	break;

    case WHITESPACE:
	goto validate_start;

    default:
	BADCHAR;
    }

    check_end (parser);
}

static INLINE void
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
c_tokenize (json_parse_t * parser)
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
validate (SV * json, unsigned int flags)
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

