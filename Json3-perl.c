/* There are two routes through the code, the PERLING route and the
   non-PERLING route. If we go via the non-PERLING route, we never
   create or alter any Perl-related stuff, we just parse each byte and
   possibly throw an error. This is for validation, to make the
   validation ultra-fast. */

#ifdef PERLING

/* We are creating Perl structures from the JSON. */

#define PREFIX(x) x
#define SVPTR SV *
#define RETURNAGAIN(x) return x
#define SETVALUE value = 

#else /* def PERLING */

/* Turn off everything to do with creating Perl things. We don't want
   any Perl memory leaks. */

#define PREFIX(x) valid_ ## x
#define SVPTR void
#define RETURNAGAIN(x) return;
#define SETVALUE 

#endif /* def PERLING */

/*#define INT_MAX_DIGITS ((int) (log (INT_MAX) / log (10)) - 1)*/

/* The maximum digits we allow an integer before throwing in the
   towel. */

#define INT_MAX_DIGITS 8

/* Turn a number into an SV. */

static INLINE SVPTR
PREFIX(number) (parser_t * parser)
{
    /* Various parsing flags. */

    /* Set to 1 if we saw ".". */

    int dot;

    /* Set to 1 if we saw "e" or "E". */

    int exp;

    /* Set to 1 if we saw "+". */

    int plus;

    /* Set to 1 if we saw a leading zero. */

    int zero;

    /* Set to 1 if we saw a leading "-". */

    int minus;

    /* Set to 1 if we saw a "-" after we saw "e" or "E". */

    int expminus;

    /* End marker for strtod/strtol. */

    char * end;

    /* Start marker for strtod/strtol. */

    char * start;

    /* A guess for integer numbers. */

    int guess;

    /* The parsed character itself, the cause of our motion. */

    char c;

    /* Set all the flags to zero. */

    dot = 0;
    exp = 0;
    plus = 0;
    zero = 0;
    minus = 0;
    expminus = 0;
    guess = 0;
    zero = 0;

    parser->end--;
    start = parser->end;

 number_start:

    switch (c = *parser->end++) {

    case '.':

	if (dot) {
	    failburger (parser, "Too many decimal points");
	}
	dot = 1;
	goto number_start;

    case '+':

	/* JSON doesn't allow a "useless plus" at the start of
	   numbers: "A number contains an integer component that may
	   be prefixed with an optional minus sign", although it does
	   allow them inside exponentials: "An exponent part begins
	   with the letter E in upper or lowercase, which may be
	   followed by a plus or minus sign." (quotes from Section 2.4
	   of RFC 4627, the JSON standard.) */

	if (! exp) {
	    failburger (parser, "Plus outside exponential");
	}

	/* Two pluses in a row, "++". */

	if (plus) {
	    failburger (parser, "Double plus");
	}
	plus = 1;
	goto number_start;
	
    case '-':

	if (exp) {
	    if (expminus) {
		failburger (parser, "Double minus in exponent");
	    }
	    expminus = 1;
	    goto number_start;
	}
	else {
	    if (minus) {
		failburger (parser, "Double minus");
	    }
	    minus = 1;
	    goto number_start;
	}
	
    case 'e':
    case 'E':

	if (exp) {
	    failburger (parser, "Doubled exponential");
	}
	exp = 1;
	goto number_start;

    case '0':

	if (! dot && ! exp) {
	    if (! guess) {
		zero = 1;
	    }
	    else {
		guess = 10 * guess;
	    }
	}
	goto number_start;

    case DIGIT19:

	if (! dot && ! exp) {
	    if (zero) {
		/* "Leading zeros are not allowed." (from section 2.4
		   of JSON standard.) */
		failburger (parser, "leading 0 in number");
	    }
	    guess = 10 * guess + (c - '0');
	}
	goto number_start;

    default:

	parser->end--;
	break;
    }

    if (dot || exp) {

	/* This is a floating point number, so we sent it to
	   "strtod". */

	double d;

	d = strtod (start, & end);
	if (end == parser->end) {
	    RETURNAGAIN (newSVnv (d));
	}
    }
    else {
	/* If the number has less than INT_MAX_DIGITS, we guess that
	   it will fit inside a Perl integer, so we don't bother doing
	   any more parsing. This is a trick to increase the speed of
	   parsing integer numbers with a small number of digits. */

	if (parser->end - start < INT_MAX_DIGITS + minus) {
	    if (minus) {
		guess = -guess;
	    }
	    RETURNAGAIN (newSViv (guess));
	}
	else {
	    int i;
	    
	    i = strtol (start, & end, 10);
	    if (end == parser->end) {
		RETURNAGAIN (newSViv (i));
	    }
	}
    }

    /* We could not convert this number using a number conversion
       routine, so we are going to convert it to a string. */

    RETURNAGAIN (newSVpv (start, parser->end - start));
}

static SVPTR
PREFIX(string) (parser_t * parser)
{
    char c;
#ifdef PERLING
    SV * string;
#endif
    int len;
    char * start;

    start = parser->end;
    len = 0;

    /* First of all, we examine the string to work out how long it is
       and to look for escapes. If we find them, we go to "bad_boys"
       and go back and do all the hard work of converting the escapes
       into the right things. If we don't find any escapes, we just
       use "start" and "len" and copy the string from inside
       "input". This is a trick to increase the speed of
       processing. */

    while (NEXTBYTE) {
	switch (c) {
	case '"':
	    goto string_end;
	case '\\':
	    goto bad_boys;
	case BADBYTES:
	    ILLEGALBYTE;
	default:
	    len++;
	}
    }
    if (parser->end - parser->input >= parser->length) {
	failburger (parser,
		    "End of input reading string starting at byte %d/%d",
		    start - parser->input, parser->length);
    }
    else {
	/* Parsing of the string ended due to a \0 byte flipping the
	   "while" switch and we dropped into this section before
	   reaching the string's end. */
	ILLEGALBYTE;
    }
 string_end:

#ifdef PERLING
    string = newSVpvn (start, len);
#endif
    goto string_done;

 bad_boys:

    parser->end = start;

    len = get_string (parser);
#ifdef PERLING
    string = newSVpvn (parser->buffer, len);
#endif

 string_done:

#ifdef PERLING
    if (parser->unicode || parser->force_unicode) {
	SvUTF8_on (string);
	parser->force_unicode = 0;
    }
#endif

    RETURNAGAIN (string);
}

/* JSON literals, a complete nuisance for people writing JSON
   parsers. */

static SVPTR
PREFIX(literal) (parser_t * parser, char c)
{
    switch (c) {
    case 't':
	if (* parser->end++ == 'r'
	    &&
	    * parser->end++ == 'u'
	    &&
	    * parser->end++ == 'e') {
#ifdef PERLING
	    SvREFCNT_inc (json_true);
#endif
	    RETURNAGAIN (json_true);
	}
	break;

    case 'n':
	if (* parser->end++ == 'u'
	    &&
	    * parser->end++ == 'l'
	    &&
	    * parser->end++ == 'l') {
#ifdef PERLING
	    SvREFCNT_inc (json_null);
#endif
	    RETURNAGAIN (json_null);
	}
	break;

    case 'f':
	if (* parser->end++ == 'a'
	    &&
	    * parser->end++ == 'l'
	    &&
	    * parser->end++ == 's'
	    &&
	    * parser->end++ == 'e') {
#ifdef PERLING
	    SvREFCNT_inc (json_false);
#endif
	    RETURNAGAIN (json_false);
	}
	break;

    default:
	/* This indicates a code failure rather than the input being
	   wrong, we should not arrive here unless the code is sending
	   wrong-looking stuff to this routine. */

	failburger (parser, "Whacko attempt to make a literal starting with %c",
		    c); 
    }
    failburger (parser, "Unparseable character %c in literal",
		parser->end - 1); 

    /* Unreached, shut up compiler warnings. */

    RETURNAGAIN (& PL_sv_undef);
}

static SVPTR PREFIX(object) (parser_t * parser);

/* Given one character, decide what to do next. This goes in the
   switch statement in both "object ()" and "array ()". */

#define PARSE(start)				\
						\
 case WHITESPACE:				\
 goto start;					\
						\
 case '"':					\
 SETVALUE PREFIX(string) (parser);		\
 break;						\
						\
 case '-':					\
 case DIGIT:					\
 SETVALUE PREFIX(number) (parser);		\
 break;						\
						\
 case '{':					\
 SETVALUE PREFIX(object) (parser);		\
 break;						\
						\
 case '[':					\
 SETVALUE PREFIX(array) (parser);		\
 break;						\
						\
 case 'f':					\
 case 'n':					\
 case 't':					\
 SETVALUE PREFIX(literal) (parser, c);	        \
 break

/* Check for illegal comma at the end of a hash/array. */

#define CHECKCOMMA						\
    if (comma) {						\
	failburger (parser, "Illegal trailing comma");		\
    }								\


/* We have seen "[", so now deal with the contents of an array. At the
   end of this routine, "parser->end" is pointing one beyond the final
   "]" of the array. */

static SVPTR
PREFIX(array) (parser_t * parser)
{
    char c;
#ifdef PERLING
    AV * av;
    SV * value;
#endif
    /* Have we seen at least one value in the array, so that commas
       are legal? */
    int comma;

    comma = 0;
#ifdef PERLING
    av = newAV ();
#endif

    /* We are either at the start of the array, just after "[", or we
       have seen at least one value, so just after ",". */

 array_start:

    switch (NEXTBYTE) {

	PARSE(array_start);

    case ']':
	CHECKCOMMA;
	/* In legal JSON, this should only be reached for an empty
	   array. */
	goto array_end;

    case ',':
	failburger (parser, "Stray comma");

    default:
	failburger (parser, "unknown character '%c' in array", c);
    }

    comma = 1;
#ifdef PERLING
    av_push (av, value);
#endif

    /* Accept either a comma or whitespace or the end of the array. */

 array_middle:

    switch (NEXTBYTE) {

    case WHITESPACE:
	goto array_middle;

    case ',':
	goto array_start;

    case ']':
	/* Array with at least one element. */
	goto array_end;

    default:
	failburger (parser, "Unknown character '%c' after object key", c);
    }

 array_end:

    RETURNAGAIN (newRV_noinc ((SV *) av));
}

/* We have seen "{", so now deal with the contents of an object. At
   the end of this routine, "parser->end" is pointing one beyond the
   final "}" of the object. */

static SVPTR
PREFIX(object) (parser_t * parser)
{
    char c;
#ifdef PERLING
    HV * hv;
    SV * value;
#endif
    string_t key;
    /* "middle" is true if we have seen ":", until the next comma. */
    int middle;
    /* "comma" is true if we have seen ",", until the next key. */
    int comma;
    /* This is set to -1 if we want a Unicode key. See "perldoc
       perlapi" under "hv_store". */
    int uniflag;

    if (parser->unicode) {
	/* Keys are unicode. */
	uniflag = -1;
    }
    else {
	/* Keys are not unicode. */
	uniflag = 1;
    }
    
    /* We have not seen a ":" yet. */
    middle = 0;
    /* We have not seen a "," yet. */
    comma = 0;

#ifdef PERLING
    hv = newHV ();
#endif

 hash_start:

    switch (NEXTBYTE) {

    case WHITESPACE:
	goto hash_start;

	/* Unreachable */

    case '}':
	CHECKCOMMA;
	goto hash_end;

	/* Unreachable */

    case '"':
	if (middle) {
	    failburger (parser, "Missing comma (,) after object value");
	}
	else {
	    comma = 0;
	    get_key_string (parser, & key);
	    goto hash_next;
	}

	/* Unreachable */

    case ',':
	if (middle) {
	    middle = 0;
	    comma = 1;
	    goto hash_start;
	}
	else {
	    failburger (parser, "Stray comma while parsing object");
	}

	/* Unreachable */

    default:
	failburger (parser, "Unknown character '%c' in object key", c);
    }

 hash_next:

    switch (NEXTBYTE) {

    case WHITESPACE:
	goto hash_next;

    case ':':
	middle = 1;
	goto hash_value;

    default:
	failburger (parser, "Unknown character '%c' after object key", c);
    }

    /* Unreachable */

 hash_value:

    switch (NEXTBYTE) {

	PARSE(hash_value);

    default:
	failburger (parser, "Unknown character '%c' in object value", c);
    }
    if (key.bad_boys) {
	int klen;
	klen = resolve_string (parser, & key);
#ifdef PERLING
	(void) hv_store (hv, parser->buffer, klen * uniflag, value, 0);
#endif
    }
    else {
#ifdef PERLING
	(void) hv_store (hv, key.start, key.length * uniflag, value, 0);
#endif
    }

    goto hash_start;

 hash_end:

    RETURNAGAIN (newRV_noinc ((SV *) hv));
}

#undef PREFIX
#undef SVPTR
#undef RETURNAGAIN
#undef SETVALUE
