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

void flatten_exprs_sql(sql* sqlast)
{
	switch(sqlast->type)
	{
		case DQL :
		{
			flatten_exprs_dql(sqlast->dql_query);
			break;
		}
		case DML :
		{
			flatten_exprs_dml(sqlast->dml_query);
			break;
		}
		case DDL :
		{
			//flatten_exprs_ddl(sqlast->ddl_query);
			break;
		}
		case TCL :
		{
			// nothing to be done here
			break;
		}
	}
}

void snprint_sql(dstring* str_p, const sql* sqlast)
{
	switch(sqlast->type)
	{
		case DQL :
		{
			snprint_dql(str_p, sqlast->dql_query);
			break;
		}
		case DML :
		{
			snprint_dml(str_p, sqlast->dml_query);
			break;
		}
		case DDL :
		{
			snprint_ddl(str_p, sqlast->ddl_query);
			break;
		}
		case TCL :
		{
			snprint_tcl(str_p, sqlast->tcl_cmd);
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
			delete_tcl(sqlast->tcl_cmd);
			break;
		}
	}
	free(sqlast);
}