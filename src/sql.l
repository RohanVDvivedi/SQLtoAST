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

"&" 					{return B_AND;}
"|" 					{return B_OR;}
"^" 					{return B_XOR;}

(?i:AND() 				{return L_AND;}
(?i:OR) 				{return L_OR;}
(?i:XOR) 				{return L_XOR;}

(?i:BETWEEN) 			{return BETWEEN;}

(?i:IN) 				{return IN;}

[-+]?[0-9]+(\.[0-9]+)?		{yylval->val.expr = new_valued_sql_expr(SQL_VAR, new_dstring(sqlget_text(yyscanner), strlen(sqlget_text(yyscanner))));
							yylval->val.type = SQL_EXPR;
							return CONST;}

[a-zA-Z_][a-zA-Z0-9_]*(\.[a-zA-Z_][a-zA-Z0-9_]*)*		{yylval->val.expr = new_valued_sql_expr(SQL_VAR, new_dstring(sqlget_text(yyscanner), strlen(sqlget_text(yyscanner))));
														yylval->val.type = SQL_EXPR;
														return VAR;}

%%
