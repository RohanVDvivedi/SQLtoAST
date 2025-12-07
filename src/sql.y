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
%parse-param { struct sql** sql_ast }

%start root

%union {
	int ival;
}

%type <ival> root

%type <ival> expr

%token <ival> NUM
%token ADD
%token SUB
%token MUL
%token DIV
%token MOD

%%

root:		expr				{$$ = $1; (*sql_ast) = malloc(sizeof(sql)); (*sql_ast)->ival = $$;}

expr:		NUM ADD NUM 		{$$ = $1 + $3; (*sql_ast) = malloc(sizeof(sql)); (*sql_ast)->ival = $$;}
			| NUM SUB NUM 		{$$ = $1 - $3; (*sql_ast) = malloc(sizeof(sql)); (*sql_ast)->ival = $$;}
			| NUM MUL NUM 		{$$ = $1 * $3; (*sql_ast) = malloc(sizeof(sql)); (*sql_ast)->ival = $$;}
			| NUM DIV NUM 		{$$ = $1 / $3; (*sql_ast) = malloc(sizeof(sql)); (*sql_ast)->ival = $$;}
			| NUM MOD NUM 		{$$ = $1 % $3; (*sql_ast) = malloc(sizeof(sql)); (*sql_ast)->ival = $$;}

%%

/* Error handling */
int sqlerror(void *scanner, struct sql** sql_ast, const char *msg) {
	fprintf(stderr, "Error: %s\n", msg);
	return 0;
}