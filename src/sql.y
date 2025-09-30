%{
#include<stdio.h>
#include<stdlib.h>

#include<sqltoast/sqltoast.h>
%}

/* Enable reentrant parser */
%define api.pure full

/* make all external symbol use sql as prefix */
%define api.prefix {sql}

%lex-param   { void* scanner }

%parse-param { void* scanner }
%parse-param { sql** sql_ast }

%start root

%union {
	int ival;
}

%type <int> root

%token <int> NUM
%token ADD

%%

root:		NUM ADD NUM 		{$$ = $1 + $3;}

%%

/* Error handling */
int sqlerror(void *scanner, const char *msg) {
	fprintf(stderr, "Error: %s\n", msg);
	return 0;
}