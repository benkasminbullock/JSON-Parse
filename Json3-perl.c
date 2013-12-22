/* The C part is broken into three pieces, "Json3-common.c",
   "Json3-perl.c", and "Json3-entry-points.c". This file contains the
   "Perl" stuff, for example if we have a string, the stuff to convert
   it into a Perl hash key or a Perl scalar is in this file. */

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

#define USEDIGIT guess = guess * 10 + (c - '0')

static INLINE SVPTR
PREFIX(number2) (parser_t * parser)
{
    /* End marker for strtod. */

    char * end;

    /* Start marker for strtod. */

    char * start;

    /* A guess for integer numbers. */

    int guess;

    /* The parsed character itself, the cause of our motion. */

    unsigned char c;

    /* If it has exp or dot in it. */

    double d;

    /* Negative number. */

    int minus;

    parser->end--;
    start = (char *) parser->end;

#define FAILNUMBER(err)				\
    parser->bad_byte = parser->end - 1;		\
    parser->error = json_error_ ## err;		\
    parser->bad_type = json_number;		\
    parser->bad_beginning =			\
	(unsigned char*) start;			\
    failbadinput (parser)

#define NUMBEREND				\
         WHITESPACE:				\
    case ']':					\
    case '}':					\
    case ','

#define XNUMBEREND (XCOMMA|XWHITESPACE|parser->end_expected)

    minus = 0;

    switch (NEXTBYTE) {
    case DIGIT19:
	guess = c - '0';
	goto leading_digit19;
    case '0':
	goto leading_zero;
    case '-':
	minus = 1;
	goto leading_minus;
    default:
	parser->expected = XDIGIT | XMINUS;
	FAILNUMBER (unexpected_character);
    }

 leading_digit19:

    switch (NEXTBYTE) {
    case DIGIT:
	USEDIGIT;
	goto leading_digit19;
    case '.':
	goto dot;
    case 'e':
    case 'E':
	goto exp;
    case NUMBEREND:
        goto int_number_end;
    default:
	parser->expected = XDIGIT | XDOT | XEXPONENTIAL | XNUMBEREND;
	FAILNUMBER (unexpected_character);
    }

 leading_zero:
    switch (NEXTBYTE) {
    case '.':
	/* "0." */
	goto dot;
    case 'e':
    case 'E':
	/* "0e" */
	goto exp;
    case NUMBEREND:
	/* "0" */
        goto int_number_end;
    default:
	parser->expected = XDOT | XEXPONENTIAL | XNUMBEREND;
	FAILNUMBER (unexpected_character);
    }

 leading_minus:
    switch (NEXTBYTE) {
    case DIGIT19:
	USEDIGIT;
	goto leading_digit19;
    case '0':
	goto leading_zero;
    default:
	parser->expected = XDIGIT;
	FAILNUMBER (unexpected_character);
    }

    /* Things like "5." are not allowed so there is no NUMBEREND
       here. */

 dot:
    switch (NEXTBYTE) {
    case DIGIT:
	goto dot_digits;
    default:
	parser->expected = XDIGIT;
	FAILNUMBER (unexpected_character);
    }

    /* We have as much as 5.5 so we can stop. */

 dot_digits:
    switch (NEXTBYTE) {
    case DIGIT:
	goto dot_digits;
    case 'e':
    case 'E':
	goto exp;
    case NUMBEREND:
        goto exp_number_end;
    default:
	parser->expected = XDIGIT | XNUMBEREND | XEXPONENTIAL;
	FAILNUMBER (unexpected_character);
    }

    /* Things like "10E" are not allowed so there is no NUMBEREND
       here. */

 exp:
    switch (NEXTBYTE) {
    case '-':
    case '+':
	goto exp_sign;
    case DIGIT:
	goto exp_digits;
    default:
	parser->expected = XDIGIT | XMINUS | XPLUS;
	FAILNUMBER (unexpected_character);
    }

 exp_sign:

    switch (NEXTBYTE) {
    case DIGIT:
	goto exp_digits;
    default:
	parser->expected = XDIGIT;
	FAILNUMBER (unexpected_character);
    }

    /* We have as much as "3.0e1" or similar. */

 exp_digits:
    switch (NEXTBYTE) {
    case DIGIT:
	goto exp_digits;
    case NUMBEREND:
        goto exp_number_end;
    default:
	parser->expected = XDIGIT | XNUMBEREND;
	FAILNUMBER (unexpected_character);
    }

 exp_number_end:
    parser->end--;
    d = strtod (start, & end);
    if ((unsigned char *) end == parser->end) {
	RETURNAGAIN (newSVnv (d));
    }
    else {
	goto string_number_end;
    }

 int_number_end:

    parser->end--;
    if (parser->end - (unsigned char *) start < INT_MAX_DIGITS + minus) {
	if (minus) {
	    guess = -guess;
	}
	RETURNAGAIN (newSViv (guess));
    }
    else {
	goto string_number_end;
    }

string_number_end:

    /* We could not convert this number using a number conversion
       routine, so we are going to convert it to a string.  This might
       happen with ridiculously long numbers or something. The JSON
       standard doesn't explicitly disallow integers with a million
       digits. */

    RETURNAGAIN (newSVpv (start, (char *) parser->end - start));
}

#if 0

/* Turn a number into an SV. */

static INLINE SVPTR
PREFIX(number) (parser_t * parser)
{
    /* Various parsing flags. */

    /* Set to 1 if we saw ".". */

    int dot;

    /* Set to 1 if we saw digits after ".". */

    int dotdigits;

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

    int newexp;

    int expdigits;

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
    dotdigits = 0;
    exp = 0;
    plus = 0;
    zero = 0;
    minus = 0;
    expminus = 0;
    guess = 0;
    zero = 0;
    newexp = 0;
    expdigits = 0;

    parser->end--;
    start = parser->end;

#define FAILNUMBER(err)				\
    parser->bad_byte = parser->end - 1;		\
    parser->error = json_error_ ## err;		\
    parser->bad_type = json_number;		\
    parser->bad_beginning = start;		\
    failbadinput (parser)

 number_start:

    switch (NEXTBYTE) {

    case '.':

	if (dot) {
	    parser->expected = XDIGIT;
	    FAILNUMBER (unexpected_character);
	}
	if (exp || (! zero && ! guess)) {
	    parser->expected = XDIGIT | XMINUS;
	    FAILNUMBER (unexpected_character);
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

	if (! newexp) {
	    parser->expected = XDIGIT;
	    FAILNUMBER (unexpected_character);
	}
	plus = 1;
	newexp = 0;
	goto number_start;
	
    case '-':

	if (exp) {
	    if (expminus || ! newexp) {
		parser->expected = XDIGIT;
		FAILNUMBER (unexpected_character);
	    }
	    expminus = 1;
	    newexp = 0;
	    goto number_start;
	}
	else {
	    /* Use "guess" as a flag that we have seen a digit. */
	    if (minus || guess || zero) {
		parser->expected = XDIGIT;
		FAILNUMBER (unexpected_character);
	    }
	    minus = 1;
	    goto number_start;
	}
	
    case 'e':
    case 'E':

	if (! zero && ! guess && ! dot) {
	    parser->expected = XDIGIT;
	    FAILNUMBER (unexpected_character);
	}
	if (dot && ! dotdigits) {
	    parser->expected = XDIGIT;
	    FAILNUMBER (unexpected_character);
	}
	if (exp) {
	    parser->expected = XDIGIT;
	    if (newexp) {
		parser->expected |= XPLUS | XMINUS;
	    }
	    FAILNUMBER (unexpected_character);
	}
	exp = 1;
	newexp = 1;
	goto number_start;

    case '0':

	if (zero && ! dot && ! exp) {
	    parser->expected = XDOT | XCOMMA | XWHITESPACE | XEXPONENTIAL | XOBJECT_END | XARRAY_END;
	    FAILNUMBER (unexpected_character);
	}
	if (exp) {
	    newexp = 0;
	    expdigits = 1;
	}
	else {
	    if (dot) {
		dotdigits = 1;
	    }
	    else {
		if (guess) {
		    guess = 10 * guess;
		}
		else {
		    zero = 1;
		}
	    }
	}
	goto number_start;

    case DIGIT19:

	if (zero && ! dot && ! exp) {
	    parser->expected = XDOT | XCOMMA | XWHITESPACE | XOBJECT_END | XARRAY_END;
	    FAILNUMBER (unexpected_character);
	}
	if (exp) {
	    newexp = 0;
	    expdigits = 1;
	}
	else {
	    if (dot) {
		dotdigits = 1;
	    }
	    else {
		guess = 10 * guess + (c - '0');
	    }
	}
	goto number_start;

    case WHITESPACE:
    case ',':
    case ']':
    case '}':

	if (minus && ! guess) {
	    parser->expected = XDIGIT;
	    FAILNUMBER (unexpected_character);
	}

	/* JSON RFC: "The E and optional sign are followed by one or
	   more digits." So if we have an E and no digits or anything
	   else, barf. */

	if ((exp && ! expdigits) ||
	    (dot && ! dotdigits)) {
	    parser->expected = XDIGIT;
	    if (newexp) {
		parser->expected |= XPLUS | XMINUS;
	    }
	    FAILNUMBER (unexpected_character);
	}

	/* We read a byte which wasn't part of a number, so reject it
	   and let the next thing to work out what to do with it. */

	parser->end--;
	break;

    default:

	if (minus && ! guess) {
	    parser->expected = XDIGIT;
	    FAILNUMBER (unexpected_character);
	}

	/* JSON RFC: "The E and optional sign are followed by one or
	   more digits." So if we have an E and no digits or anything
	   else, barf. */

	if ((exp && ! expdigits) || (dot && ! dotdigits)) {
	    parser->expected = XDIGIT;
	    if (newexp) {
		parser->expected |= XPLUS | XMINUS;
	    }
	    FAILNUMBER (unexpected_character);
	}
	else {
	    /* This looks a bit strange but we can accept a comma or
	       whitespace or the end of an object or array as the end of a
	       number. */
	    parser->expected = XCOMMA | XWHITESPACE;
	    if (zero) {
		if (dot) {
		    parser->expected |= XDIGIT;
		}
		else {
		    if (exp) {
			/* 0e0 is legal */
			parser->expected |= XDIGIT;
		    }
		    else {
			/* 0e is legal */
			parser->expected |= XEXPONENTIAL;
		    }		    
		}
	    }
	    else {
		parser->expected |= XDIGIT;
	    }
	    if ((guess || dotdigits) && ! exp) {
		parser->expected |= XEXPONENTIAL;
	    }
	    if (! exp && ! dot && (zero || guess)) {
		parser->expected |= XDOT;
	    }
	    if (newexp) {
		parser->expected |= XPLUS | XMINUS;
	    }
	    FAILNUMBER (unexpected_character);
	}
    }

    if (dot || exp) {

	/* This is a floating point number, so we send it to
	   "strtod". */

	double d;
	d = strtod (start, & end);
	if (end == parser->end) {
	    RETURNAGAIN (newSVnv (d));
	}
	else {
	    /* Oops, Marc Lehmann was right all along. */
	    failbug (__FILE__, __LINE__, parser,
		     "unable to parse '%.*s' with strtod",
		     parser->end - start, start);
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
    }

    /* We could not convert this number using a number conversion
       routine, so we are going to convert it to a string.  This might
       happen with ridiculously long numbers or something. The JSON
       standard doesn't explicitly disallow integers with a million
       digits. */

    RETURNAGAIN (newSVpv (start, parser->end - start));
}

#endif /* 0 */

static SVPTR
PREFIX(string) (parser_t * parser)
{
    unsigned char c;
#ifdef PERLING
    SV * string;
#endif
    int len;
    unsigned char * start;

    start = parser->end;
    len = 0;

    /* First of all, we examine the string to work out how long it is
       and to look for escapes. If we find them, we go to "contains_escapes"
       and go back and do all the hard work of converting the escapes
       into the right things. If we don't find any escapes, we just
       use "start" and "len" and copy the string from inside
       "input". This is a trick to increase the speed of
       processing. */

 string_start:
    switch (NEXTBYTE) {
    case '"':
	goto string_end;
    case '\\':
	goto contains_escapes;
    case BADBYTES:
	ILLEGALBYTE;
#define ADDBYTE len++
#include "utf8-byte-one.c"
	
    default:
	ILLEGALBYTE;
    }
    /* Parsing of the string ended due to a \0 byte flipping the
       "while" switch and we dropped into this section before
       reaching the string's end. */
    ILLEGALBYTE;

#include "utf8-next-byte.c"
#undef ADDBYTE

 string_end:

#ifdef PERLING
    string = newSVpvn ((char *) start, len);
#endif
    goto string_done;

 contains_escapes:

    parser->end = start;

    len = get_string (parser);
#ifdef PERLING
    string = newSVpvn ((char *) parser->buffer, len);
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

#define FAILLITERAL(c)					\
    parser->expected = XIN_LITERAL;			\
    parser->literal_char = c;				\
    parser->bad_beginning = start;			\
    parser->error = json_error_unexpected_character;	\
    parser->bad_type = json_literal;			\
    parser->bad_byte = parser->end - 1;			\
    failbadinput (parser)

static SVPTR
PREFIX(literal_true) (parser_t * parser)
{
    unsigned char * start;
    start = parser->end - 1;
    if (* parser->end++ == 'r') {
	if (* parser->end++ == 'u') {
	    if (* parser->end++ == 'e') {
#ifdef PERLING
		SvREFCNT_inc (json_true);
#endif
		RETURNAGAIN (json_true);
	    }
	    FAILLITERAL('e');
	}
	FAILLITERAL('u');
    }
    FAILLITERAL('r');

    /* Unreached, shut up compiler warnings. */

    RETURNAGAIN (& PL_sv_undef);
}

static SVPTR
PREFIX(literal_false) (parser_t * parser)
{
    unsigned char * start;
    start = parser->end - 1;
    if (* parser->end++ == 'a') {
	if (* parser->end++ == 'l') {
	    if (* parser->end++ == 's') {
		if (* parser->end++ == 'e') {
#ifdef PERLING
		    SvREFCNT_inc (json_false);
#endif
		    RETURNAGAIN (json_false);
		}
		FAILLITERAL('e');
	    }
	    FAILLITERAL('s');
	}
	FAILLITERAL('l');
    }
    FAILLITERAL('a');

    /* Unreached, shut up compiler warnings. */

    RETURNAGAIN (& PL_sv_undef);
}

static SVPTR
PREFIX(literal_null) (parser_t * parser)
{
    unsigned char * start;
    start = parser->end - 1;
    if (* parser->end++ == 'u') {
	if (* parser->end++ == 'l') {
	    if (* parser->end++ == 'l') {
#ifdef PERLING
		SvREFCNT_inc (json_null);
#endif
		RETURNAGAIN (json_null);
	    }
	    FAILLITERAL('l');
	}
	FAILLITERAL('l');
    }
    FAILLITERAL('u');

    /* Unreached, shut up compiler warnings. */

    RETURNAGAIN (& PL_sv_undef);
}

static SVPTR PREFIX(object) (parser_t * parser);

/* Given one character, decide what to do next. This goes in the
   switch statement in both "object ()" and "array ()". */

#define PARSE(start,expected)			\
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
 parser->end_expected = expected;	        \
 SETVALUE PREFIX(number2) (parser);		\
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
 SETVALUE PREFIX(literal_false) (parser);	\
 break;			                        \
						\
 case 'n':					\
 SETVALUE PREFIX(literal_null) (parser);	\
 break;			                        \
						\
 case 't':					\
 SETVALUE PREFIX(literal_true) (parser);	\
 break

#define FAILARRAY(err)				\
    parser->bad_byte = parser->end - 1;		\
    parser->bad_type = json_array;		\
    parser->bad_beginning = start;		\
    parser->error = json_error_ ## err;		\
    failbadinput (parser)

/* We have seen "[", so now deal with the contents of an array. At the
   end of this routine, "parser->end" is pointing one beyond the final
   "]" of the array. */

static SVPTR
PREFIX(array) (parser_t * parser)
{
    unsigned char c;
    unsigned char * start;
#ifdef PERLING
    AV * av;
    SV * value;
#endif

#ifdef PERLING
    av = newAV ();
#endif
    start = parser->end - 1;

 array_start:

    switch (NEXTBYTE) {

	PARSE (array_start,XARRAY_END);

    case ']':
	goto array_end;

    default:
	parser->expected = VALUE_START | XWHITESPACE | XARRAY_END;
	FAILARRAY(unexpected_character);
    }

#ifdef PERLING
    av_push (av, value);
#endif

    /* Accept either a comma or whitespace or the end of the array. */

 array_middle:

    switch (NEXTBYTE) {

    case WHITESPACE:
	goto array_middle;

    case ',':
	goto array_next;

    case ']':
	/* Array with at least one element. */
	goto array_end;

    default:

	parser->expected = XWHITESPACE | XCOMMA | XARRAY_END;
	FAILARRAY(unexpected_character);
    }

 array_next:

    switch (NEXTBYTE) {

	PARSE(array_next, XARRAY_END);

    default:
	parser->expected = VALUE_START | XWHITESPACE;
	FAILARRAY(unexpected_character);
    }

#ifdef PERLING
    av_push (av, value);
#endif

    goto array_middle;

 array_end:

    RETURNAGAIN (newRV_noinc ((SV *) av));
}

#define FAILOBJECT(err)				\
    parser->bad_byte = parser->end - 1;		\
    parser->bad_type = json_object;		\
    parser->bad_beginning = start;		\
    parser->error = json_error_ ## err;		\
    failbadinput (parser)

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
    /* This is set to -1 if we want a Unicode key. See "perldoc
       perlapi" under "hv_store". */
    int uniflag;
    /* Start of parsing. */
    unsigned char * start;

    start = parser->end - 1;

    if (parser->unicode) {
	/* Keys are unicode. */
	uniflag = -1;
    }
    else {
	/* Keys are not unicode. */
	uniflag = 1;
    }

#ifdef PERLING
    hv = newHV ();
#endif

 hash_start:

    switch (NEXTBYTE) {
    case WHITESPACE:
	goto hash_start;
    case '}':
	goto hash_end;
    case '"':
	get_key_string (parser, & key);
	goto hash_next;
    default:
	parser->expected = XWHITESPACE | XSTRING_START | XOBJECT_END;
	FAILOBJECT(unexpected_character);
    }

 hash_middle:

    switch (NEXTBYTE) {
    case WHITESPACE:
	goto hash_middle;
    case '}':
	goto hash_end;
    case ',':
	goto hash_key;
    default:
	parser->expected = XWHITESPACE | XCOMMA | XOBJECT_END;
	FAILOBJECT(unexpected_character);
    }

 hash_key:

    switch (NEXTBYTE) {
    case WHITESPACE:
	goto hash_key;
    case '"':
	get_key_string (parser, & key);
	goto hash_next;
    default:
	parser->expected = XWHITESPACE | XSTRING_START;
	FAILOBJECT(unexpected_character);
    }

 hash_next:

    switch (NEXTBYTE) {
    case WHITESPACE:
	goto hash_next;
    case ':':
	goto hash_value;
    default:
	parser->expected = XWHITESPACE | XVALUE_SEPARATOR;
	FAILOBJECT(unexpected_character);
    }

    /* Unreachable */

 hash_value:

    switch (NEXTBYTE) {
	PARSE(hash_value, XOBJECT_END);
    default:
	parser->expected = XWHITESPACE | VALUE_START;
	FAILOBJECT(unexpected_character);
    }

    if (key.contains_escapes) {
	int klen;
	klen = resolve_string (parser, & key);
#ifdef PERLING
	(void) hv_store (hv, (char *) parser->buffer, klen * uniflag, value, 0);
#endif
    }
    else {
#ifdef PERLING
	(void) hv_store (hv, (char *) key.start, key.length * uniflag, value, 0);
#endif
    }
    goto hash_middle;

 hash_end:

    RETURNAGAIN (newRV_noinc ((SV *) hv));
}

#undef PREFIX
#undef SVPTR
#undef RETURNAGAIN
#undef SETVALUE
