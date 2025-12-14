%option reentrant bison-bridge noyywrap
%option prefix="sql"
%option header-file="./src/sql.yy.h"

%{
#include<cutlery/stream.h>
#include"sql.tab.h"

#define YY_INPUT(buf, result, max_size) {                                                \
	int read_error = 0;                                                                  \
	result = read_from_stream(yyget_extra(yyscanner), buf, max_size, &read_error);       \
	if(read_error)                                                                       \
		yyterminate();                                                                   \
}

#define YYSTYPE SQLSTYPE
%}

%option extra-type="stream *"

%%

[ \t\n\r]+      		{;}

"("						{return OPEN_BRACKET;}
")"						{return CLOSE_BRACKET;}

"-" 					{return NEG;}
"~" 					{return B_NOT;}
(?i:NOT) 				{return L_NOT;}

"+" 					{return ADD;}
"*" 					{return MUL;}
"/" 					{return DIV;}
"%" 					{return MOD;}

">" 					{return GT;}
">=" 					{return GTE;}
"<" 					{return LT;}
"<=" 					{return LTE;}
"=" 					{return EQ;}
"<>" 					{return NEQ;}
"!=" 					{return NEQ;}

"&" 					{return B_AND;}
"|" 					{return B_OR;}
"^" 					{return B_XOR;}

"<<" 					{return L_SHIFT;}
">>" 					{return R_SHIFT;}

"||" 					{return CONCAT;}
(?i:LIKE) 				{return LIKE;}

(?i:AND) 				{return L_AND;}
(?i:OR) 				{return L_OR;}
(?i:XOR) 				{return L_XOR;}

(?i:IS) 				{return IS;}

(?i:BETWEEN) 			{return BETWEEN;}

(?i:IN) 				{return IN;}
","						{return COMMA;}

(?i:CAST)				{return CAST;}
(?i:AS)					{return AS;}

(?i:TRUE)				{yylval->val.expr = new_const_non_valued_sql_expr(SQL_TRUE);
						yylval->val.type = SQL_EXPR;
						return TRUE;}
(?i:FALSE)				{yylval->val.expr = new_const_non_valued_sql_expr(SQL_FALSE);
						yylval->val.type = SQL_EXPR;
						return FALSE;}
(?i:UNKNOWN)			{yylval->val.expr = new_const_non_valued_sql_expr(SQL_UNKNOWN);
						yylval->val.type = SQL_EXPR;
						return UNKNOWN;}
(?i:NULL)				{yylval->val.expr = new_const_non_valued_sql_expr(SQL_NULL);
						yylval->val.type = SQL_EXPR;
						return _NULL_;}

(?i:BOOL)				{return BOOL;}
(?i:BOOLEAN)			{return BOOL;}
(?i:BIT)				{return BIT;}
(?i:SMALLINT)			{return SMALLINT;}
(?i:INT)				{return INT;}
(?i:INTEGER)			{return INT;}
(?i:BIGINT)				{return BIGINT;}
(?i:REAL)				{return REAL;}
(?i:DOUBLE)				{return DOUBLE;}
(?i:FLOAT)				{return FLOAT;}
(?i:DECIMAL)			{return DECIMAL;}
(?i:NUMERIC)			{return NUMERIC;}
(?i:TEXT)				{return TEXT;}
(?i:CHAR)				{return CHAR;}
(?i:CHARACTER)			{return CHAR;}
(?i:VARCHAR)			{return VARCHAR;}
(?i:CLOB)				{return CLOB;}
(?i:BLOB)				{return BLOB;}
(?i:DATE)				{return DATE;}
(?i:TIME)				{return TIME;}
(?i:TIMESTAMP)			{return TIMESTAMP;}

(?i:WITH[[:space:]]+TIME[[:space:]]+ZONE)						{return WITH_TZ;}
(?i:WITHOUT[[:space:]]+TIME[[:space:]]+ZONE)					{return WITHOUT_TZ;}

[-+]?[0-9]+(\.[0-9]+)?([eE][0-9]+)?		{yylval->val.expr = new_valued_sql_expr(SQL_NUM, new_dstring(sqlget_text(yyscanner), strlen(sqlget_text(yyscanner))));
										yylval->val.type = SQL_EXPR;
										return NUMBER;}

'[^']*'									{yylval->val.expr = new_valued_sql_expr(SQL_STR, new_dstring(sqlget_text(yyscanner) + 1, strlen(sqlget_text(yyscanner))-2));
										yylval->val.type = SQL_EXPR;
										return STRING;}

[a-zA-Z_][a-zA-Z0-9_]*(\.[a-zA-Z_][a-zA-Z0-9_]*)*		{yylval->val.expr = new_valued_sql_expr(SQL_VAR, new_dstring(sqlget_text(yyscanner), strlen(sqlget_text(yyscanner))));
														yylval->val.type = SQL_EXPR;
														return IDENTIFIER;}

%%
