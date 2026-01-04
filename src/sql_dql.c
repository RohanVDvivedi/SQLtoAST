#include<sqltoast/sql_dql.h>

void destroydql(sql_dql* dql)
{
	delete_sql_expr(dql->where_expr);
}