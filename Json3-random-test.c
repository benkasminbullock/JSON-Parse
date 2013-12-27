/*
     _                 _____                       _                 
    | |___  ___  _ __ |___ /   _ __ __ _ _ __   __| | ___  _ __ ___  
 _  | / __|/ _ \| '_ \  |_ \  | '__/ _` | '_ \ / _` |/ _ \| '_ ` _ \ 
| |_| \__ \ (_) | | | |___) | | | | (_| | | | | (_| | (_) | | | | | |
 \___/|___/\___/|_| |_|____/  |_|  \__,_|_| |_|\__,_|\___/|_| |_| |_|
                                                                     
 _            _   
| |_ ___  ___| |_ 
| __/ _ \/ __| __|
| ||  __/\__ \ |_ 
 \__\___||___/\__|
                  
*/

/*

  This is a torture test. The main entry point is the function
  "random_json". "Random_json" creates a string containing all-null
  bytes, and feeds it to "c_validate" in the file
  "Json3-entry-points.c". "C_validate" responds with an error message
  containing a list of what bytes it considered acceptable or
  unacceptable at the point where parsing failed,
  "parser->valid_bytes". The routine "alter_one_byte" below then
  changes the offending final byte into each byte from 0 to MAXBYTE,
  and sends the resulting string to the parser again.

  "Alter_one_byte" then checks that the list of valid and invalid
  bytes is correct. If the byte was supposed to be invalid and the
  parser allows it, or if the byte was supposed to be valid and the
  parser disallows it, it prints an error and halts.

  If the invalid byte wasn't detected, the error is either "no bad
  byte in parser" if no error occurred, or if an error occurred, but
  not at the location of the invalid byte, "Failed to detect error in
  <type> with bad byte", followed by the error itself, followed by the
  pointer and byte contents. 

  Conversely, if a valid byte was rejected, the error message given is
  printed as in "Got error <error message> with supposedly valid
  value" plus the pointer and byte details and the error message.

  Various code useful for tracking down the exact error is in the
  file but commented out since it creates a huge amount of screen
  noise. To locate and switch on this debugging code, search this
  file for lines of the form "#if 0" and set to "#if 1" to get the
  specific type of debugging offered.

  "Alter_one_byte" also keeps a list of the valid bytes in "choose",
  and after it has successfully tested the valid and invalid bytes,
  it then randomly chooses one of the valid bytes, and lengthens the
  string by one valid byte, with a zero byte after that. This new
  longer string is then sent to the parser again, and the process of
  testing the consistency of the error messages with valid and
  invalid bytes is repeated.

  There are three ways "random_json" can halt without exiting:

  1. The string overflows the buffer length,

  In this case, "random_json" merely returns without printing a
  message.

  2. The string is a valid JSON string,

  In this case "random_json" finishes with a message "Success in
  parsing."

  3. The string contains a surrogate pair error.

  It's virtually impossible to fix a surrogate pair error by byte
  fiddling, so in the rare cases this happens, "random_json" simply
  prints a message "Unfixable error" and returns.

*/

#define INITIALLENGTH 0x1000

/* Print one character in the most readable format. */

static void
print_json_char (unsigned char c)
{
    printf ("%c", c);
    return;
    if (isprint (c)) {
	if (c == ' ') {
	    printf (" ");
	}
	else {
	    printf ("%c", c);
	}
    }
    else {
	switch (c) {
	case '\r':
	    printf ("\\r");
	    break;
	case '\t':
	    printf ("\\t");
	    break;
	case '\f':
	    printf ("\\f");
	    break;
	case '\n':
	    printf ("\\n");
	    break;
	default:
	    printf ("\\X%02X", c);
	}
    }
}

/* Print out the JSON in "parser". */

static void
print_json (parser_t * parser)
{
    int i;
    unsigned char * json;
    json = parser->input;
    printf ("JSON is now: ");
    for (i = 0; i < parser->length; i++) {
	print_json_char (json[i]);
    }
    printf ("\n");
}

/* Reset the parser to the initial state. */

static void
reset_parser (parser_t * parser)
{
    /* Reset the parser to start from the beginning. */
    parser->end = parser->input;
    parser->expected = 0;
    parser->error = json_error_invalid;
    parser->bad_byte = 0;
    parser->last_byte = parser->input + parser->length;
}

#define SURROPAIRFAIL					\
    if ((parser->error ==				\
	 json_error_not_surrogate_pair) ||		\
	(parser->bad_type == json_unicode_escape)) {	\
	printf ("Unfixable error.\n");			\
	goto end;					\
    }

/* Alter the final byte and run again, and see if an error is
   produced. */

static int
alter_one_byte (parser_t * parser)
{
    int i;
    int valid_bytes[MAXBYTE];
    int choose[MAXBYTE];
    int n_choose;
    unsigned char * expected_bad_byte;

    n_choose = 0;

    if (! parser->bad_byte) {
	fprintf (stderr, "no bad byte in parser.\n");
	exit (1);
    }

    expected_bad_byte = parser->bad_byte;
    //    printf ("%p %p %p %p\n", expected_bad_byte, parser->input, parser->last_byte, parser->end);

    /* Copy the list of valid bytes from the parser, because the valid
       byte list in the parser will be overwritten with each
       successive call to "c_validate". */

    for (i = 0; i < MAXBYTE; i++) {
	valid_bytes[i] = parser->valid_bytes[i];
    }

    for (i = 0; i < MAXBYTE; i++) {
	if (! valid_bytes[i]) {
#if 0
	    /* Notify of doing an invalid byte test. */
	    printf ("I don't think that ");
	    print_json_char (i);
	    printf (" is OK\n");
#endif
	    /* Change the byte to the supposedly invalid value. Use
	       "expected_bad_byte" because the parser's bad_byte is
	       reset each time. */
	    //printf ("Setting value to %d\n", i);
	    * expected_bad_byte = i;
	    //		print_json (parser);
	    reset_parser (parser);
	    if (setjmp (parser->biscuit)) {
		/* Failed. */
		if (parser->bad_byte != expected_bad_byte) {
		    SURROPAIRFAIL;
		    fprintf (stderr, "Failed to detect error in %s"
			     " with bad byte ",
			     type_names[parser->bad_type]);
		    print_json_char (*expected_bad_byte);
		    printf (":\n");
		    print_json (parser);
		    fprintf (stderr,
			     "parser->bad_byte=%d "
			     "should be %d with byte %d (0x%02X)"
			     ", error was %s.\n",
			     parser->bad_byte - parser->input,
			     expected_bad_byte - parser->input, i, i,
			     parser->last_error);
		    exit (EXIT_FAILURE);
		}
		else {
		    //		    printf ("Bad byte as expected.\n");
		}
	    }
	    else {
		/* The above "if" statement is like the a "catch" and
		   this "else" statement is like a "try" in
		   JavaScript. Alternatively, this "else" is a Perl
		   "eval" and the above if branch is like "if ($@)" in
		   Perl. */
		c_validate (parser);
	    }
	}
	else {
#if 0
	    printf ("I think that ");
	    print_json_char (i);
	    printf (" is OK\n");
#endif
	    choose[n_choose] = i;
	    n_choose++;
	    if (setjmp (parser->biscuit)) {
		if (parser->bad_byte == expected_bad_byte) {
		    print_json (parser);
		    fprintf (stderr,
			     "Got error %s to %p with supposedly "
			     "valid value wanted %p with byte %d.\n",
			     parser->last_error, parser->bad_byte,
			     expected_bad_byte, i);
		    exit (EXIT_FAILURE);
		}
	    }
	    else {
		/* Change the byte to the supposedly valid value. Use
		   "expected_bad_byte" because the parser's bad_byte is
		   reset each time. */
		//		printf ("Setting value to valid value %d\n", i);
		* expected_bad_byte = i;
		//		print_json (parser->input, parser->length);
		reset_parser (parser);
		c_validate (parser);
	    }
	}
    }
    
    /* Finally, stuff a random byte into the thing. */
 nouniescapes:
    * expected_bad_byte = choose[random () % n_choose];
    if (* expected_bad_byte == 'u' && * (expected_bad_byte - 1) == '\\') {
	/* Temporarily ban \u because of endless hassles with
	   surrogate pairs. */
	goto nouniescapes;
    }
#if 0
    printf ("Chossing byte '%c' %X\n", * expected_bad_byte, * expected_bad_byte);
#endif
    /* We now have one more valid byte, and only the last byte is
       questionable. */
    parser->length++;
    return 0;
 end:
    /* Bail out, we have generated an unfixable string e.g. a
       surrogate pair with the second half broken. */
    return -1;
}

/* Make a string of completely random yet valid JSON. This
   torture-tests the parser by checking that its marking of valid and
   invalid bytes is correct. */

static int
random_json ()
{
    unsigned char * json;
    int json_size;
    int json_length;
    int i;
    parser_t parser_o = {0};
    parser_t * parser;

    parser = & parser_o;
    json_size = INITIALLENGTH;
    json = malloc (json_size);
    if (! json) {
	fprintf (stderr, "%s:%d: out of memory.\n", __FILE__, __LINE__);
	exit (EXIT_FAILURE);
    }
    json_length = 0;
    for (i = 0; i < json_size; i++) {
	json[i] = 0;
    }
    parser_o.input = json;
    parser_o.randomtest = 1;
    for (i = 1; i < json_size; i++) {
	reset_parser (& parser_o);
	json_length = i;
	parser_o.length = json_length;
	parser_o.last_byte = parser_o.input + parser_o.length;
#if 0
	printf ("Attempting to parse length %d\n", i);
	print_json (& parser_o);
#endif
	if (setjmp (parser_o.biscuit)) {
#if 0
	    int j;
#endif
	    /*
	    if (parser_o.error == json_error_unexpected_character &&
		parser_o.bad_byte == parser_o.input) {
		printf ("Failed on first byte with unexpected character.\n");
	    }
	    */
#if 0
	    printf ("Got error: %s\n", parser_o.last_error);
#endif
	    if (parser_o.error == json_error_unexpected_character) {
#if 0
		for (j = 0; j < MAXBYTE; j++) {
		    printf ("%d", parser_o.valid_bytes[j]);
		    if (j % 0x40 == 0x3F) {
			printf ("\n");
		    }
		}
#endif
		/* Alter bytes and run again. */
		if (alter_one_byte (& parser_o)) {
		    /* Trap for unfixable errors. */
		    goto end;
		}
		//print_json (& parser_o);
	    }
	    else if (parser_o.error == json_error_empty_input) {
#if 0
		printf ("empty input error.\n");
#endif
		parser_o.bad_byte = parser_o.input;
		alter_one_byte (& parser_o);
#if 0
		printf ("Expanding string to\n");
		print_json (& parser_o);
		printf ("here aga.\n");
#endif
	    }
	    else if (parser_o.error == json_error_unexpected_end_of_input) {
#if 0
		printf ("end of input.\n");
#endif
		parser_o.bad_byte = parser_o.input + json_length - 1;
#if 0
		printf ("Altering final byte.\n");
#endif
		alter_one_byte (& parser_o);
	    }
	    else {
		SURROPAIRFAIL;
		print_json (& parser_o);
		fprintf (stderr, "error: %s.\n", parser_o.last_error);
		exit (1);
	    }
	}
	else {
	    c_validate (& parser_o);
	    print_json (& parser_o);
	    printf ("Success in parsing.\n");
	    //	    exit (0);
	    break;
	}
    }
 end:
    parser_free (& parser_o);
    free (json);
    return parser_o.length * MAXBYTE;
}

