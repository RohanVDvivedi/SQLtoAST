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

(?i:EXISTS)				{return EXISTS;}

(?i:CAST)				{return CAST;}
(?i:AS)					{return AS;}

(?i:ANY)				{return ANY;}
(?i:SOME)				{return ANY;}
(?i:ALL)				{return ALL;}

(?i:TRUE)				{return TRUE;}
(?i:FALSE)				{return FALSE;}
(?i:UNKNOWN)			{return UNKNOWN;}
(?i:NULL)				{return _NULL_;}

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

(?i:INTERSECT)			{return INTERSECT;}
(?i:UNION)				{return UNION;}
(?i:EXCEPT)				{return EXCEPT;}
(?i:DISTINCT)			{return DISTINCT;}

(?i:SELECT)				{return SELECT;}
(?i:UNIQUE)				{return UNIQUE;}
(?i:FROM)				{return FROM;}
(?i:JOIN)				{return JOIN;}
(?i:ON)					{return ON;}
(?i:WHERE)				{return WHERE;}
(?i:GROUP)				{return GROUP;}
(?i:BY)					{return BY;}
(?i:HAVING)				{return HAVING;}
(?i:ORDER)				{return ORDER;}
(?i:LIMIT)				{return LIMIT;}
(?i:OFFSET)				{return OFFSET;}

(?i:USING) 				{return USING;}
(?i:INNER) 				{return INNER;}
(?i:OUTER)				{return OUTER;}
(?i:LEFT)				{return LEFT;}
(?i:FULL)				{return FULL;}
(?i:CROSS)				{return CROSS;}
(?i:LATERAL)			{return LATERAL;}
(?i:NATURAL)			{return NATURAL;}

(?i:ASC)				{return ASC;}
(?i:DESC)				{return DESC;}

(?i:UPDATE)				{return UPDATE;}
(?i:DEFAULT)			{return DEFAULT;}

(?i:DELETE)				{return DELETE;}

(?i:START)				{return START;}
(?i:BEGIN)				{return _BEGIN_;}
(?i:TRANSACTION)		{return TRANSACTION;}
(?i:ROLLBACK)			{return ROLLBACK;}
(?i:COMMIT)				{return COMMIT;}
(?i:RELEASE)			{return RELEASE;}
(?i:TO)					{return TO;}
(?i:SAVEPOINT)			{return SAVEPOINT;}
(?i:WORK)				{return WORK;}
(?i:SET)				{return SET;}
(?i:CHARACTERISTICS)	{return CHARACTERISTICS;}

(?i:READ)				{return READ;}
(?i:WRITE)				{return WRITE;}
(?i:ONLY)				{return ONLY;}
(?i:ISOLATION[[:space:]]+LEVEL)	{return ISOLATION_LEVEL;}
(?i:UNCOMMITTED)		{return UNCOMMITTED;}
(?i:COMMITTED)			{return COMMITTED;}
(?i:REPEATABLE)			{return REPEATABLE;}
(?i:SERIALIZABLE)		{return SERIALIZABLE;}

(?i:WITH[[:space:]]+TIME[[:space:]]+ZONE)						{return WITH_TZ;}
(?i:WITHOUT[[:space:]]+TIME[[:space:]]+ZONE)					{return WITHOUT_TZ;}

[-+]?[0-9]+														{yylval->sval = new_dstring(sqlget_text(yyscanner), strlen(sqlget_text(yyscanner))); return INTEGER;}

[-+]?[0-9]+(\.[0-9]+)?([eE][0-9]+)?								{yylval->sval = new_dstring(sqlget_text(yyscanner), strlen(sqlget_text(yyscanner))); return NUMBER;}

'[^']*'															{yylval->sval = new_dstring(sqlget_text(yyscanner) + 1, strlen(sqlget_text(yyscanner))-2); return STRING;}

[a-zA-Z_][a-zA-Z0-9_]*(\.[a-zA-Z_][a-zA-Z0-9_]*)*(\.\*)?		{yylval->sval = new_dstring(sqlget_text(yyscanner), strlen(sqlget_text(yyscanner))); return IDENTIFIER;}

%%
