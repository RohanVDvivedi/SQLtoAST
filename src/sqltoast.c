#include<sqltoast/sqltoast.h>

#include<sql.tab.h>
#define YYSTYPE SQLSTYPE
#include<sql.yy.h>

sql* parse_sql(stream* strm, int* error)
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

void print_sql(const sql* sqlast)
{
	switch(sqlast->type)
	{
		case DQL :
		{
			print_dql(sqlast->dql_query, 0);
			break;
		}
		case DML :
		{
			//print_dml(sqlast->dml_query);
			break;
		}
		case DDL :
		{
			//print_ddl(sqlast->ddl_query);
			break;
		}
		case TCL :
		{
			//print_tcl(sqlast->tcl_query);
			break;
		}
	}
}

void delete_sql(sql* sqlast)
{
	switch(sqlast->type)
	{
		case DQL :
		{
			delete_dql(sqlast->dql_query);
			break;
		}
		case DML :
		{
			delete_dml(sqlast->dml_query);
			break;
		}
		case DDL :
		{
			delete_ddl(sqlast->ddl_query);
			break;
		}
		case TCL :
		{
			delete_tcl(sqlast->tcl_query);
			break;
		}
	}
	free(sqlast);
}