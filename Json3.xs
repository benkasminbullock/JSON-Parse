#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "Json3-perl.c"

MODULE=Json3 PACKAGE=Json3

PROTOTYPES: ENABLE

BOOT:
	Json3_error_handler = perl_error_handler;

