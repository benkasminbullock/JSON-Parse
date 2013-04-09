#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "Json3-perl.c"

MODULE=Json3 PACKAGE=Json3

PROTOTYPES: ENABLE

BOOT:
{
	json_true = get_sv ("Json3::true", GV_ADD);
	json_false = get_sv ("Json3::false", GV_ADD);
	json_null = get_sv ("Json3::null", GV_ADD);
}

SV * parse_json (json)
	SV * json;
CODE:
	RETVAL = parse (json);
OUTPUT:
	RETVAL
