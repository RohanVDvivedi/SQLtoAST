%{
#include<stdio.h>
#include<stdlib.h>

#include<sqltoast/sqltoast.h>
#include<sqltoast/sql_expression.h>

typedef enum value_type value_type;
enum value_type
{
	SQL_ROOT,
	SQL_EXPR,
};

typedef struct value value;
struct value
{
	value_type type;
	union
	{
		sql* root;
		sql_expression* expr;
	};
};

#define MAKE_ROOT(root) ((value){.type = SQL_ROOT, .root = root})
#define MAKE_EXPR(expr) ((value){.type = SQL_EXPR, .expr = expr})

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
	value val;
}

%type <ival> root

%type <ival> expr

%token <ival> NUM

%token NEG
%token B_NOT
%token L_NOT

%token ADD
%token MUL
%token DIV
%token MOD

%token GT
%token GTE
%token LT
%token LTE
%token EQ
%token NEQ

%token B_AND
%token B_OR,
%token B_XOR

%token L_AND
%token L_OR
%token L_XOR

#token BETWEEN

%token IN

%token CONST

%token VAR

%%

root:		expr				{$$ = $1; (*sql_ast)->ival = $$;}

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