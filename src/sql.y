%{
#include<stdio.h>
#include<stdlib.h>

#include<sqltoast/sqltoast.h>
#include<sqltoast/sql_expression.h>
%}

%code requires {
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
}

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

%type <val> root

%type <val> expr

%token <val> VAR
%token <val> CONST

%token OPEN_BRACKET
%token CLOSE_BRACKET

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
%token B_OR
%token B_XOR

%token L_AND
%token L_OR
%token L_XOR

%token BETWEEN

%token IN

/* Precedence + Associativity */
/* Lowest precedence first (so Bison gives them the lowest binding) */
%left L_OR
%left L_XOR
%left L_AND

%left BETWEEN

%left EQ NEQ GT GTE LT LTE

%left B_OR
%left B_XOR
%left B_AND

%left ADD NEG
%left MUL DIV MOD

/* Highest: unary operators */
%right L_NOT
%right B_NOT
%right UMINUS UPLUS

%%

root:		expr						{(*sql_ast) = malloc(sizeof(sql)); (*sql_ast)->expr = $1.expr;}

expr:		OPEN_BRACKET expr CLOSE_BRACKET 		{$$ = $2;}
			|  VAR 									{$$ = $1;}
			| CONST									{$$ = $1;}

			| B_NOT expr 							{$$.expr = new_unary_sql_expr(SQL_BITNOT, $2.expr); $$.type = SQL_EXPR;}
			| NEG expr %prec UMINUS					{$$.expr = new_unary_sql_expr(SQL_NEG, $2.expr); $$.type = SQL_EXPR;}
			| ADD expr %prec UPLUS					{$$ = $2;}
			| L_NOT expr 							{$$.expr = new_unary_sql_expr(SQL_LOGNOT, $2.expr); $$.type = SQL_EXPR;}

			| expr MUL expr 						{$$.expr = new_binary_sql_expr(SQL_MUL, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| expr DIV expr 						{$$.expr = new_binary_sql_expr(SQL_DIV, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| expr MOD expr 						{$$.expr = new_binary_sql_expr(SQL_MOD, $1.expr, $3.expr); $$.type = SQL_EXPR;}

			| expr ADD expr 						{$$.expr = new_binary_sql_expr(SQL_ADD, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| expr NEG expr 						{$$.expr = new_binary_sql_expr(SQL_SUB, $1.expr, $3.expr); $$.type = SQL_EXPR;}

			| expr B_AND expr 						{$$.expr = new_binary_sql_expr(SQL_BITAND, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| expr B_OR expr 						{$$.expr = new_binary_sql_expr(SQL_BITOR, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| expr B_XOR expr 						{$$.expr = new_binary_sql_expr(SQL_BITXOR, $1.expr, $3.expr); $$.type = SQL_EXPR;}

			| expr EQ expr							{$$.expr = new_binary_sql_expr(SQL_EQ, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| expr NEQ expr 						{$$.expr = new_binary_sql_expr(SQL_NEQ, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| expr GT expr 							{$$.expr = new_binary_sql_expr(SQL_GT, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| expr GTE expr 						{$$.expr = new_binary_sql_expr(SQL_GTE, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| expr LT expr 							{$$.expr = new_binary_sql_expr(SQL_LT, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| expr LTE expr 						{$$.expr = new_binary_sql_expr(SQL_LTE, $1.expr, $3.expr); $$.type = SQL_EXPR;}

			| expr BETWEEN expr L_AND expr 			{$$.expr = new_between_sql_expr($1.expr, $3.expr, $5.expr); $$.type = SQL_EXPR;}

			| expr L_AND expr 						{$$.expr = new_binary_sql_expr(SQL_LOGAND, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| expr L_OR expr 						{$$.expr = new_binary_sql_expr(SQL_LOGOR, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| expr L_XOR expr 						{$$.expr = new_binary_sql_expr(SQL_LOGXOR, $1.expr, $3.expr); $$.type = SQL_EXPR;}

%%

/* Error handling */
int sqlerror(void *scanner, struct sql** sql_ast, const char *msg) {
	fprintf(stderr, "Error: %s\n", msg);
	return 0;
}