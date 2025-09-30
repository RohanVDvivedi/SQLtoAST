%option reentrant noyywrap
%option bison-bridge
%option prefix="sql"
%option header-file="sql.yy.h"

%{
#include<cutlery/stream.h>
#include"sql.tab.h"

#define YY_INPUT(buf, result, max_size) {                                                \
	int read_error = 0;                                                                  \
	result = read_from_stream(yyget_extra(yyscanner), buf, max_size, &read_error);       \
	if(read_error)                                                                       \
		yyterminate();                                                                   \
}
%}

%option extra-type="stream *"

%%

[0-9]+					{yylval->ival = atoi(yytext); return NUM;}

"+" 					{return ADD;}

%%
