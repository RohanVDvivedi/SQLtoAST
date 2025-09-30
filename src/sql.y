%{
#include<stdio.h>
#include<stdlib.h>

/* Forward declare scanner type from Flex */
struct yyscan_t;
%}

/* Enable reentrant parser */
%define api.pure full

/* make all external symbol use sql as prefix */
%define api.prefix {sql}

%parse-param { sql** sql_ast }

%start root

%%

%%

/* Error handling */
int parsererror(void *scanner, const char *msg) {
	fprintf(stderr, "Error: %s\n", msg);
	return 0;
}