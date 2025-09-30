%{
#include<stdio.h>
#include<stdlib.h>

#include<sqltoast/sqltoast.h>

/* Forward declare scanner type from Flex */
struct sql_yyscan_t;
%}

/* Enable reentrant parser */
%define api.pure full

/* make all external symbol use sql as prefix */
%define api.prefix {sql}

%parse-param { sql** sql_ast }
%parse-param { sql_yyscan_t *scanner }

%start root

%%

root:
	;

%%

/* Error handling */
int sqlerror(void *scanner, const char *msg) {
	fprintf(stderr, "Error: %s\n", msg);
	return 0;
}