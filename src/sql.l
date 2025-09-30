%option reentrant noyywrap
%option bison-bridge
%option prefix="sql"

%{
#include<cutlery/stream.h>
#include"sql.tab.h"   /* Bison’s token definitions */
%}

%option extra-type="stream *"

#define YY_INPUT(buf, result, max_size) {
	int read_error = 0;
	result = read_from_stream((yyget_extra(yyscanner), buf, max_size, &read_error);
	if(read_error)
		yyterminate();
}

%%

%%
