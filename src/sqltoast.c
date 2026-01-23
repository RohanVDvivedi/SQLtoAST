#include<sqltoast/sqltoast.h>

#include<sql.tab.h>
#define YYSTYPE SQLSTYPE
#include<sql.yy.h>

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
	{
		(*error) = result;
		return NULL;
	}
}

void destroysql(sql* sqlast)
{
	switch(sqlast->type)
	{
		case DQL :
		{
			destroydql(sqlast->dql_query);
			break;
		}
		case DML :
		{
			destroydml(sqlast->dml_query);
			break;
		}
		case DDL :
		{
			destroyddl(sqlast->ddl_query);
			break;
		}
		case TCL :
		{
			destroytcl(sqlast->tcl_query);
			break;
		}
	}
	free(sqlast);
}