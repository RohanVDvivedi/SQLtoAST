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

%type <val> expr

%token <val> NUM
%token <val> CONST

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

root:		expr						{(*sql_ast) = malloc(sizeof(sql_ast)); (*sql_ast)->expr = $1;}

expr:		| VAR 						{$$ = $1;}
			| CONST						{$$ = $1;}

			| expr ADD expr 			{$$->expr = new_binary_sql_expr(SQL_ADD, $1->expr, $3->expr); $$->type = SQL_EXPR}
			| expr NEG expr 			{$$->expr = new_binary_sql_expr(SQL_SUB, $1->expr, $3->expr); $$->type = SQL_EXPR}
			| expr MUL expr 			{$$->expr = new_binary_sql_expr(SQL_MUL, $1->expr, $3->expr); $$->type = SQL_EXPR}
			| expr DIV expr 			{$$->expr = new_binary_sql_expr(SQL_DIV, $1->expr, $3->expr); $$->type = SQL_EXPR}
			| expr MOD expr 			{$$->expr = new_binary_sql_expr(SQL_MOD, $1->expr, $3->expr); $$->type = SQL_EXPR}

			| expr GT expr 				{$$->expr = new_binary_sql_expr(SQL_GT, $1->expr, $3->expr); $$->type = SQL_EXPR}
			| expr GTE expr 			{$$->expr = new_binary_sql_expr(SQL_GTE, $1->expr, $3->expr); $$->type = SQL_EXPR}
			| expr LT expr 				{$$->expr = new_binary_sql_expr(SQL_LT, $1->expr, $3->expr); $$->type = SQL_EXPR}
			| expr LTE expr 			{$$->expr = new_binary_sql_expr(SQL_LTE, $1->expr, $3->expr); $$->type = SQL_EXPR}
			| expr EQ expr				{$$->expr = new_binary_sql_expr(SQL_EQ, $1->expr, $3->expr); $$->type = SQL_EXPR}
			| expr NEQ expr 			{$$->expr = new_binary_sql_expr(SQL_NEQ, $1->expr, $3->expr); $$->type = SQL_EXPR}

			| expr B_AND expr 			{$$->expr = new_binary_sql_expr(SQL_BITAND, $1->expr, $3->expr); $$->type = SQL_EXPR}
			| expr B_OR expr 			{$$->expr = new_binary_sql_expr(SQL_BITOR, $1->expr, $3->expr); $$->type = SQL_EXPR}
			| expr B_XOR expr 			{$$->expr = new_binary_sql_expr(SQL_BITXOR, $1->expr, $3->expr); $$->type = SQL_EXPR}

			| expr L_AND expr 			{$$->expr = new_binary_sql_expr(SQL_LOGAND, $1->expr, $3->expr); $$->type = SQL_EXPR}
			| expr L_OR expr 			{$$->expr = new_binary_sql_expr(SQL_LOGOR, $1->expr, $3->expr); $$->type = SQL_EXPR}
			| expr L_XOR expr 			{$$->expr = new_binary_sql_expr(SQL_LOGXOR, $1->expr, $3->expr); $$->type = SQL_EXPR}

			| NEG expr 					{$$->expr = new_unary_sql_expr(SQL_NEG, $2->expr); $$->type = SQL_EXPR}
			| B_NOT expr 				{$$->expr = new_unary_sql_expr(SQL_BITNOT, $2->expr); $$->type = SQL_EXPR}
			| L_NOT expr 				{$$->expr = new_unary_sql_expr(SQL_LOGNOT, $2->expr); $$->type = SQL_EXPR}

			| BETWEEN expr L_AND expr 	{$$->expr = new_between_sql_expr($2->expr, $4->expr); $$->type = SQL_EXPR}

%%

/* Error handling */
int sqlerror(void *scanner, struct sql** sql_ast, const char *msg) {
	fprintf(stderr, "Error: %s\n", msg);
	return 0;
}