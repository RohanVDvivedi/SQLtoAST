#include<sqltoast/sql_dql.h>

#include<stdlib.h>

sql_dql* new_dql()
{
	sql_dql* dql = malloc(sizeof(sql_dql));

	dql->type = SELECT_QUERY;

	dql->where_expr = NULL;
	dql->having_expr = NULL;
	dql->offset_expr = NULL;
	dql->limit_expr = NULL;

	return dql;
}

void destroydql(sql_dql* dql)
{
	if(dql->where_expr)
		delete_sql_expr(dql->where_expr);

	if(dql->having_expr)
		delete_sql_expr(dql->having_expr);

	if(dql->offset_expr)
		delete_sql_expr(dql->offset_expr);

	if(dql->limit_expr)
		delete_sql_expr(dql->limit_expr);

	free(dql);
}