%{
#include<stdio.h>
#include<stdlib.h>

#include<sqltoast/sqltoast.h>
#include<sqltoast/sql_expression.h>
%}

%code requires {
	#include<sqltoast/sqltoast.h>
	#include<sqltoast/sql_expression.h>
	#include<sqltoast/sql_type.h>

	typedef enum value_type value_type;
	enum value_type
	{
		SQL_ROOT,
		SQL_EXPR,
		SQL_EXPR_LIST,
		SQL_TYPE,
	};

	typedef struct value value;
	struct value
	{
		value_type type;
		union
		{
			sql* root;
			sql_expression* expr;
			arraylist expr_list;
			sql_type data_type;
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
	dstring sval; // for any other lexeme
}

%type <val> root

%type <val> any_expr
%type <val> bool_expr
%type <val> bool_literal
%type <val> value_expr
%type <val> value_expr_list
%type <val> type
%type <val> type_name
%type <val> type_specs
%type <sval> type_spec
%type <val> type_with_or_without_timezone

%token <val> IDENTIFIER
%token <val> NUMBER
%token <val> STRING
%token <val> TRUE
%token <val> FALSE
%token <val> UNKNOWN
%token <val> _NULL_

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

%token L_SHIFT
%token R_SHIFT

%token CONCAT
%token LIKE

%token IS

%token BETWEEN

%token IN
%token COMMA

%token CAST
%token AS

/* sql type names and needed tokens */

%token BOOL

%token BIT

%token SMALLINT
%token INT
%token BIGINT

%token REAL
%token DOUBLE
%token FLOAT

%token DECIMAL
%token NUMERIC

%token TEXT
%token CHAR
%token VARCHAR
%token CLOB
%token BLOB

%token DATE
%token TIME
%token TIMESTAMP

%token WITH_TZ
%token WITHOUT_TZ

/* Precedence + Associativity */


/* Lowest precedence first (so Bison gives them the lowest binding) */

%left L_OR
%left L_XOR
%left L_AND

%nonassoc BETWEEN_PREC

%nonassoc IN_PREC

%nonassoc IS_PREC

%left EQ NEQ GT GTE LT LTE LIKE_PREC

%left B_OR
%left B_XOR
%left B_AND

%left L_SHIFT R_SHIFT

%left CONCAT

%left ADD NEG
%left MUL DIV MOD

/* Highest: unary operators */
%right L_NOT
%right B_NOT
%right UMINUS UPLUS

%%

root:
			any_expr							{(*sql_ast) = malloc(sizeof(sql)); (*sql_ast)->expr = $1.expr;}

any_expr :
			bool_expr							{$$ = $1;}
			| value_expr						{$$ = $1;}

bool_expr :
			OPEN_BRACKET bool_expr CLOSE_BRACKET 											{$$ = $2;}

			| IDENTIFIER																	{$$ = $1;}
			| TRUE																			{$$ = $1;}
			| FALSE																			{$$ = $1;}
			| _NULL_																		{$$ = $1;}
			| UNKNOWN																		{$$ = $1;}

			| L_NOT bool_expr 																{$$.expr = new_unary_sql_expr(SQL_LOGNOT, $2.expr); $$.type = SQL_EXPR;}

			| value_expr EQ value_expr														{$$.expr = new_binary_sql_expr(SQL_EQ, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| value_expr NEQ value_expr 													{$$.expr = new_binary_sql_expr(SQL_NEQ, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| value_expr GT value_expr 														{$$.expr = new_binary_sql_expr(SQL_GT, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| value_expr GTE value_expr 													{$$.expr = new_binary_sql_expr(SQL_GTE, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| value_expr LT value_expr 														{$$.expr = new_binary_sql_expr(SQL_LT, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| value_expr LTE value_expr 													{$$.expr = new_binary_sql_expr(SQL_LTE, $1.expr, $3.expr); $$.type = SQL_EXPR;}

			| value_expr LIKE value_expr %prec LIKE_PREC									{$$.expr = new_binary_sql_expr(SQL_LIKE, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| value_expr L_NOT LIKE value_expr %prec LIKE_PREC								{$$.expr = new_unary_sql_expr(SQL_LOGNOT, new_binary_sql_expr(SQL_LIKE, $1.expr, $4.expr)); $$.type = SQL_EXPR;}

			| value_expr IN OPEN_BRACKET value_expr_list CLOSE_BRACKET %prec IN_PREC		{$$.expr = new_in_sql_expr($1.expr, $4.expr_list); $$.type = SQL_EXPR;}
			| value_expr L_NOT IN OPEN_BRACKET value_expr_list CLOSE_BRACKET %prec IN_PREC	{$$.expr = new_unary_sql_expr(SQL_LOGNOT, new_in_sql_expr($1.expr, $5.expr_list)); $$.type = SQL_EXPR;}

			| value_expr BETWEEN value_expr L_AND value_expr %prec BETWEEN_PREC				{$$.expr = new_between_sql_expr($1.expr, $3.expr, $5.expr); $$.type = SQL_EXPR;}
			| value_expr L_NOT BETWEEN value_expr L_AND value_expr %prec BETWEEN_PREC		{$$.expr = new_unary_sql_expr(SQL_LOGNOT, new_between_sql_expr($1.expr, $4.expr, $6.expr)); $$.type = SQL_EXPR;}

			| any_expr IS bool_literal %prec IS_PREC										{$$.expr = new_binary_sql_expr(SQL_IS, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| any_expr IS L_NOT bool_literal %prec IS_PREC 									{$$.expr = new_unary_sql_expr(SQL_LOGNOT, new_binary_sql_expr(SQL_IS, $1.expr, $4.expr)); $$.type = SQL_EXPR;}

			| bool_expr L_AND bool_expr 													{$$.expr = new_binary_sql_expr(SQL_LOGAND, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| bool_expr L_OR bool_expr 														{$$.expr = new_binary_sql_expr(SQL_LOGOR, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| bool_expr L_XOR bool_expr 													{$$.expr = new_binary_sql_expr(SQL_LOGXOR, $1.expr, $3.expr); $$.type = SQL_EXPR;}

bool_literal:
			TRUE																	{$$ = $1;}
			| FALSE																	{$$ = $1;}
			| _NULL_																{$$ = $1;}
			| UNKNOWN																{$$ = $1;}

value_expr :
			OPEN_BRACKET value_expr CLOSE_BRACKET 									{$$ = $2;}
			| NUMBER																{$$ = $1;}
			| STRING																{$$ = $1;}
			| IDENTIFIER															{$$ = $1;}
			| TRUE																	{$$ = $1;}
			| FALSE																	{$$ = $1;}
			| _NULL_																{$$ = $1;}
			| UNKNOWN																{$$ = $1;}

			| B_NOT value_expr 														{$$.expr = new_unary_sql_expr(SQL_BITNOT, $2.expr); $$.type = SQL_EXPR;}
			| NEG value_expr %prec UMINUS											{$$.expr = new_unary_sql_expr(SQL_NEG, $2.expr); $$.type = SQL_EXPR;}
			| ADD value_expr %prec UPLUS											{$$ = $2;}

			| value_expr MUL value_expr 											{$$.expr = new_binary_sql_expr(SQL_MUL, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| value_expr DIV value_expr 											{$$.expr = new_binary_sql_expr(SQL_DIV, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| value_expr MOD value_expr 											{$$.expr = new_binary_sql_expr(SQL_MOD, $1.expr, $3.expr); $$.type = SQL_EXPR;}

			| value_expr ADD value_expr 											{$$.expr = new_binary_sql_expr(SQL_ADD, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| value_expr NEG value_expr 											{$$.expr = new_binary_sql_expr(SQL_SUB, $1.expr, $3.expr); $$.type = SQL_EXPR;}

			| value_expr B_AND value_expr 											{$$.expr = new_binary_sql_expr(SQL_BITAND, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| value_expr B_OR value_expr 											{$$.expr = new_binary_sql_expr(SQL_BITOR, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| value_expr B_XOR value_expr 											{$$.expr = new_binary_sql_expr(SQL_BITXOR, $1.expr, $3.expr); $$.type = SQL_EXPR;}

			| value_expr L_SHIFT value_expr 										{$$.expr = new_binary_sql_expr(SQL_LSHIFT, $1.expr, $3.expr); $$.type = SQL_EXPR;}
			| value_expr R_SHIFT value_expr 										{$$.expr = new_binary_sql_expr(SQL_RSHIFT, $1.expr, $3.expr); $$.type = SQL_EXPR;}

			| value_expr CONCAT value_expr 											{$$.expr = new_binary_sql_expr(SQL_CONCAT, $1.expr, $3.expr); $$.type = SQL_EXPR;}

			| IDENTIFIER OPEN_BRACKET value_expr_list CLOSE_BRACKET					{$$.expr = new_func_sql_expr($1.expr->value, $3.expr_list); free($1.expr); $$.type = SQL_EXPR;}

			| CAST OPEN_BRACKET value_expr AS type CLOSE_BRACKET 					{$$.expr = new_cast_sql_expr($3.expr, $4.type); $$.type = SQL_EXPR;}

value_expr_list :
			value_expr 																{initialize_expr_list(&($$.expr_list)); insert_in_expr_list(&($$.expr_list), $1.expr); $$.type = SQL_EXPR_LIST;}
			| value_expr_list COMMA value_expr 										{insert_in_expr_list(&($1.expr_list), $3.expr); $$ = $1; $$.type = SQL_EXPR_LIST;}

type : type_name type_specs type_with_or_without_timezone							{$$.data_type = $1.data_type; $$.data_type.specs_size = $2.data_type.specs_size; memcpy($$.data_type.specs, $2.data_type.specs, sizeof($$.data_type.specs)); $$.data_type.with_time_zone = $$.data_type.with_time_zone; $$.type = SQL_TYPE;}

type_name : 	BOOL 					{$$.data_type.type_name = SQL_BOOL; $$.type = SQL_TYPE;}
				| BIT 					{$$.data_type.type_name = SQL_BIT; $$.type = SQL_TYPE;}
				| SMALLINT 				{$$.data_type.type_name = SQL_SMALLINT; $$.type = SQL_TYPE;}
				| INT 					{$$.data_type.type_name = SQL_INT; $$.type = SQL_TYPE;}
				| BIGINT 				{$$.data_type.type_name = SQL_BIGINT; $$.type = SQL_TYPE;}
				| REAL 					{$$.data_type.type_name = SQL_REAL; $$.type = SQL_TYPE;}
				| DOUBLE 				{$$.data_type.type_name = SQL_DOUBLE; $$.type = SQL_TYPE;}
				| FLOAT 				{$$.data_type.type_name = SQL_FLOAT; $$.type = SQL_TYPE;}
				| DECIMAL 				{$$.data_type.type_name = SQL_DECIMAL; $$.type = SQL_TYPE;}
				| NUMERIC 				{$$.data_type.type_name = SQL_NUMERIC; $$.type = SQL_TYPE;}
				| TEXT 					{$$.data_type.type_name = SQL_TEST; $$.type = SQL_TYPE;}
				| CHAR 					{$$.data_type.type_name = SQL_CHAR; $$.type = SQL_TYPE;}
				| VARCHAR 				{$$.data_type.type_name = SQL_VARCHAR; $$.type = SQL_TYPE;}
				| CLOB 					{$$.data_type.type_name = SQL_CLOB; $$.type = SQL_TYPE;}
				| BLOB 					{$$.data_type.type_name = SQL_BLOB; $$.type = SQL_TYPE;}
				| DATE 					{$$.data_type.type_name = SQL_DATE; $$.type = SQL_TYPE;}
				| TIME 					{$$.data_type.type_name = SQL_TIME; $$.type = SQL_TYPE;}
				| TIMESTAMP 			{$$.data_type.type_name = SQL_TIMESTAMP; $$.type = SQL_TYPE;}

type_specs : 																						{$$.data_type.specs_size = 0; $$.type = SQL_TYPE;}
				| OPEN_BRACKET spec CLOSE_BRACKET 													{$$.data_type.specs[0] = $1; $$.data_type.specs_size = 1; $$.type = SQL_TYPE;}
				| OPEN_BRACKET spec COMMA spec CLOSE_BRACKET 										{$$.data_type.specs[0] = $1; $$.data_type.specs[1] = $1; $$.data_type.specs_size = 2; $$.type = SQL_TYPE;}
				| OPEN_BRACKET spec COMMA spec COMMA spec CLOSE_BRACKET 							{$$.data_type.specs[0] = $1; $$.data_type.specs[1] = $1; $$.data_type.specs[2] = $1; $$.data_type.specs_size = 3; $$.type = SQL_TYPE;}
				| OPEN_BRACKET spec COMMA spec COMMA spec COMMA spec CLOSE_BRACKET 					{$$.data_type.specs[0] = $1; $$.data_type.specs[1] = $1; $$.data_type.specs[2] = $1; $$.data_type.specs[3] = $1; $$.data_type.specs_size = 4; $$.type = SQL_TYPE;}
				| OPEN_BRACKET spec COMMA spec COMMA spec COMMA spec COMMA spec CLOSE_BRACKET 		{$$.data_type.specs[0] = $1; $$.data_type.specs[1] = $1; $$.data_type.specs[2] = $1; $$.data_type.specs[3] = $1; $$.data_type.specs[4] = $1; $$.data_type.specs_size = 5; $$.type = SQL_TYPE;}

type_with_or_without_timezone : 						{$$.data_type.with_time_zone = 0; $$.type = SQL_TYPE;}
									| WITH_TZ 			{$$.data_type.with_time_zone = 1; $$.type = SQL_TYPE;}
									| WITHOUT_TZ 		{$$.data_type.with_time_zone = 0; $$.type = SQL_TYPE;}

%%

/* Error handling */
int sqlerror(void *scanner, struct sql** sql_ast, const char *msg) {
	fprintf(stderr, "Error: %s\n", msg);
	return 0;
}