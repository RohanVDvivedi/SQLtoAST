#include<sqltoast/sql_dql.h>

#include<stdlib.h>

void destroydql(sql_dql* dql)
{
	delete_sql_expr(dql->where_expr);

	free(dql);
}