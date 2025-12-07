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

[0-9]+					{yylval->ival = atoi(sqlget_text(yyscanner)); return NUM;}

"+" 					{return ADD;}
"-" 					{return SUB;}
"*" 					{return MUL;}
"/" 					{return DIV;}
"%" 					{return MOD;}

%%
