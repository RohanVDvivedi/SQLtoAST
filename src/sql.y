%{
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sqltoast/sqltoast.h>
#include<sqltoast/sql_expression.h>
%}

%code requires {
	#include<sqltoast/sqltoast.h>
	#include<sqltoast/sql_expression.h>
	#include<sqltoast/sql_type.h>
}

/* Enable reentrant parser */
%define api.pure full

/* make all external symbol use sql as prefix */
%define api.prefix {sql}

%lex-param   { void* scanner }

%parse-param { void* scanner }
%parse-param { struct sql** sql_ast }

%start sql_query

%union {
	sql* sql_query;

	sql_dql* dql_query;

	sql_expression* expr;

	arraylist ptr_list;

	sql_type data_type;

	uint64_t uval;
	int64_t ival;

	dstring sval; // for any other lexeme
}

%type <sql_query> sql_query

/* SELECT query */
%type <dql_query> dql_query

%type <expr> expr
%type <expr> bool_expr
%type <expr> bool_literal
%type <expr> value_expr
%type <ptr_list> value_expr_list

%type <data_type> type
%type <data_type> type_name
%type <data_type> type_specs
%type <uval> type_spec
%type <data_type> type_with_or_without_timezone

%token <sval> IDENTIFIER
%token <sval> INTEGER
%token <sval> NUMBER
%token <sval> STRING

%token TRUE
%token FALSE
%token UNKNOWN
%token _NULL_

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

sql_query:
			dql_query 							{(*sql_ast) = malloc(sizeof(sql)); (*sql_ast)->type = DQL; (*sql_ast)->dql_query = $1;}

dql_query:
			expr 								{$$ = malloc(sizeof(sql_dql)); $$->type = SELECT_QUERY; $$->where_expr = $1;}

expr :
			bool_expr							{$$ = $1;}
			| value_expr						{$$ = $1;}

bool_expr :
			OPEN_BRACKET bool_expr CLOSE_BRACKET 											{$$ = $2;}

			| IDENTIFIER																	{$$ = new_valued_sql_expr(SQL_VAR, $1);}
			| TRUE																			{$$ = new_const_non_valued_sql_expr(SQL_TRUE);}
			| FALSE																			{$$ = new_const_non_valued_sql_expr(SQL_FALSE);}
			| _NULL_																		{$$ = new_const_non_valued_sql_expr(SQL_NULL);}
			| UNKNOWN																		{$$ = new_const_non_valued_sql_expr(SQL_UNKNOWN);}

			| L_NOT bool_expr 																{$$ = new_unary_sql_expr(SQL_LOGNOT, $2);}

			| value_expr EQ value_expr														{$$ = new_binary_sql_expr(SQL_EQ, $1, $3);}
			| value_expr NEQ value_expr 													{$$ = new_binary_sql_expr(SQL_NEQ, $1, $3);}
			| value_expr GT value_expr 														{$$ = new_binary_sql_expr(SQL_GT, $1, $3);}
			| value_expr GTE value_expr 													{$$ = new_binary_sql_expr(SQL_GTE, $1, $3);}
			| value_expr LT value_expr 														{$$ = new_binary_sql_expr(SQL_LT, $1, $3);}
			| value_expr LTE value_expr 													{$$ = new_binary_sql_expr(SQL_LTE, $1, $3);}

			| value_expr LIKE value_expr %prec LIKE_PREC									{$$ = new_binary_sql_expr(SQL_LIKE, $1, $3);}
			| value_expr L_NOT LIKE value_expr %prec LIKE_PREC								{$$ = new_unary_sql_expr(SQL_LOGNOT, new_binary_sql_expr(SQL_LIKE, $1, $4));}

			| value_expr IN OPEN_BRACKET value_expr_list CLOSE_BRACKET %prec IN_PREC		{$$ = new_in_sql_expr($1, $4);}
			| value_expr L_NOT IN OPEN_BRACKET value_expr_list CLOSE_BRACKET %prec IN_PREC	{$$ = new_unary_sql_expr(SQL_LOGNOT, new_in_sql_expr($1, $5));}

			| value_expr BETWEEN value_expr L_AND value_expr %prec BETWEEN_PREC				{$$ = new_between_sql_expr($1, $3, $5);}
			| value_expr L_NOT BETWEEN value_expr L_AND value_expr %prec BETWEEN_PREC		{$$ = new_unary_sql_expr(SQL_LOGNOT, new_between_sql_expr($1, $4, $6));}

			| expr IS bool_literal %prec IS_PREC											{$$ = new_binary_sql_expr(SQL_IS, $1, $3);}
			| expr IS L_NOT bool_literal %prec IS_PREC 										{$$ = new_unary_sql_expr(SQL_LOGNOT, new_binary_sql_expr(SQL_IS, $1, $4));}

			| bool_expr L_AND bool_expr 													{$$ = new_binary_sql_expr(SQL_LOGAND, $1, $3);}
			| bool_expr L_OR bool_expr 														{$$ = new_binary_sql_expr(SQL_LOGOR, $1, $3);}
			| bool_expr L_XOR bool_expr 													{$$ = new_binary_sql_expr(SQL_LOGXOR, $1, $3);}

bool_literal:
			TRUE				{$$ = new_const_non_valued_sql_expr(SQL_TRUE);}
			| FALSE				{$$ = new_const_non_valued_sql_expr(SQL_FALSE);}
			| _NULL_			{$$ = new_const_non_valued_sql_expr(SQL_NULL);}
			| UNKNOWN			{$$ = new_const_non_valued_sql_expr(SQL_UNKNOWN);}

value_expr :
			OPEN_BRACKET value_expr CLOSE_BRACKET 									{$$ = $2;}
			| INTEGER 																{$$ = new_valued_sql_expr(SQL_NUM, $1);}
			| NUMBER																{$$ = new_valued_sql_expr(SQL_NUM, $1);}
			| STRING																{$$ = new_valued_sql_expr(SQL_STR, $1);}
			| IDENTIFIER															{$$ = new_valued_sql_expr(SQL_VAR, $1);}
			| TRUE																	{$$ = new_const_non_valued_sql_expr(SQL_TRUE);}
			| FALSE																	{$$ = new_const_non_valued_sql_expr(SQL_FALSE);}
			| _NULL_																{$$ = new_const_non_valued_sql_expr(SQL_NULL);}
			| UNKNOWN																{$$ = new_const_non_valued_sql_expr(SQL_UNKNOWN);}

			| B_NOT value_expr 														{$$ = new_unary_sql_expr(SQL_BITNOT, $2);}
			| NEG value_expr %prec UMINUS											{$$ = new_unary_sql_expr(SQL_NEG, $2);}
			| ADD value_expr %prec UPLUS											{$$ = $2;}

			| value_expr MUL value_expr 											{$$ = new_binary_sql_expr(SQL_MUL, $1, $3);}
			| value_expr DIV value_expr 											{$$ = new_binary_sql_expr(SQL_DIV, $1, $3);}
			| value_expr MOD value_expr 											{$$ = new_binary_sql_expr(SQL_MOD, $1, $3);}

			| value_expr ADD value_expr 											{$$ = new_binary_sql_expr(SQL_ADD, $1, $3);}
			| value_expr NEG value_expr 											{$$ = new_binary_sql_expr(SQL_SUB, $1, $3);}

			| value_expr B_AND value_expr 											{$$ = new_binary_sql_expr(SQL_BITAND, $1, $3);}
			| value_expr B_OR value_expr 											{$$ = new_binary_sql_expr(SQL_BITOR, $1, $3);}
			| value_expr B_XOR value_expr 											{$$ = new_binary_sql_expr(SQL_BITXOR, $1, $3);}

			| value_expr L_SHIFT value_expr 										{$$ = new_binary_sql_expr(SQL_LSHIFT, $1, $3);}
			| value_expr R_SHIFT value_expr 										{$$ = new_binary_sql_expr(SQL_RSHIFT, $1, $3);}

			| value_expr CONCAT value_expr 											{$$ = new_binary_sql_expr(SQL_CONCAT, $1, $3);}

			| IDENTIFIER OPEN_BRACKET value_expr_list CLOSE_BRACKET					{$$ = new_func_sql_expr($1, $3);}

			| CAST OPEN_BRACKET value_expr AS type CLOSE_BRACKET 					{$$ = new_cast_sql_expr($3, $5);}

value_expr_list :
			value_expr 																{initialize_expr_list(&($$)); insert_in_expr_list(&($$), $1);}
			| value_expr_list COMMA value_expr 										{insert_in_expr_list(&($1), $3); $$ = $1;}

type : type_name type_specs type_with_or_without_timezone							{$$ = $1; $$.spec_size = $2.spec_size; memcpy($$.spec, $2.spec, sizeof($$.spec)); $$.with_time_zone = $3.with_time_zone;}

type_name : 	BOOL 					{$$.type_name = SQL_BOOL;}
				| BIT 					{$$.type_name = SQL_BIT;}
				| SMALLINT 				{$$.type_name = SQL_SMALLINT;}
				| INT 					{$$.type_name = SQL_INT;}
				| BIGINT 				{$$.type_name = SQL_BIGINT;}
				| REAL 					{$$.type_name = SQL_REAL;}
				| DOUBLE 				{$$.type_name = SQL_DOUBLE;}
				| FLOAT 				{$$.type_name = SQL_FLOAT;}
				| DECIMAL 				{$$.type_name = SQL_DECIMAL;}
				| NUMERIC 				{$$.type_name = SQL_NUMERIC;}
				| TEXT 					{$$.type_name = SQL_TEXT;}
				| CHAR 					{$$.type_name = SQL_CHAR;}
				| VARCHAR 				{$$.type_name = SQL_VARCHAR;}
				| CLOB 					{$$.type_name = SQL_CLOB;}
				| BLOB 					{$$.type_name = SQL_BLOB;}
				| DATE 					{$$.type_name = SQL_DATE;}
				| TIME 					{$$.type_name = SQL_TIME;}
				| TIMESTAMP 			{$$.type_name = SQL_TIMESTAMP;}

type_specs : 																												{$$.spec_size = 0;}
				| OPEN_BRACKET type_spec CLOSE_BRACKET 																		{$$.spec[0] = $2; $$.spec_size = 1;}
				| OPEN_BRACKET type_spec COMMA type_spec CLOSE_BRACKET 														{$$.spec[0] = $2; $$.spec[1] = $4; $$.spec_size = 2;}
				| OPEN_BRACKET type_spec COMMA type_spec COMMA type_spec CLOSE_BRACKET 										{$$.spec[0] = $2; $$.spec[1] = $4; $$.spec[2] = $6; $$.spec_size = 3;}
				| OPEN_BRACKET type_spec COMMA type_spec COMMA type_spec COMMA type_spec CLOSE_BRACKET 						{$$.spec[0] = $2; $$.spec[1] = $4; $$.spec[2] = $6; $$.spec[3] = $8; $$.spec_size = 4;}
				| OPEN_BRACKET type_spec COMMA type_spec COMMA type_spec COMMA type_spec COMMA type_spec CLOSE_BRACKET 		{$$.spec[0] = $2; $$.spec[1] = $4; $$.spec[2] = $6; $$.spec[3] = $8; $$.spec[4] = $10; $$.spec_size = 5;}

type_spec : INTEGER  			{unsigned long long int res = 0; get_unsigned_long_long_int_from_dstring(&($1), 10, &res); $$ = res; deinit_dstring(&($1));}

type_with_or_without_timezone : 						{$$.with_time_zone = 0;}
									| WITH_TZ 			{$$.with_time_zone = 1;}
									| WITHOUT_TZ 		{$$.with_time_zone = 0;}

%%

/* Error handling */
int sqlerror(void *scanner, struct sql** sql_ast, const char *msg) {
	fprintf(stderr, "Error: %s\n", msg);
	return 0;
}