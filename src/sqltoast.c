#include<sqltoast/sqltoast.h>

#define YYSTYPE SQLSTYPE
#include<sql.yy.h>
#include<sql.tab.h>

sql* parsesql(stream* strm, int* error)
{
	sql* res = NULL;

	yyscan_t scanner;
	if(sqllex_init_extra(strm, &scanner))
    	return NULL;

	int result = sqlparse(scanner, &res);

	sqllex_destroy(scanner);

	if(result == 0)
		return res;
	else
		return NULL;
}